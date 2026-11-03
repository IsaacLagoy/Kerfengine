#include <kerf/engine/resource/SoundServer.h>

#include "engine/resource/SoundVoice.h"

#include <kerf/shared/Const.h>
#include <algorithm>
#include <filesystem>
#include <iostream>
#include <stdexcept>

namespace kerf {

namespace {

// Play to end of file
constexpr ma_uint64 TO_END = ~static_cast<ma_uint64>(0);

ma_engine engine;
bool initialized = false;

[[noreturn]] void fail(const std::string& message)
{
    throw std::runtime_error(ANSI_RED + "[SoundServer] " + message + ANSI_RESET);
}

void requireInit()
{
    if (!initialized)
    {
        fail("not initialized, create an Engine first");
    }
}

ma_uint64 toFrames(float seconds, ma_uint32 sampleRate)
{
    if (seconds <= 0.0f)
    {
        return 0;
    }
    return static_cast<ma_uint64>(static_cast<double>(seconds) * sampleRate);
}

// Extra plays after the first
int loopsLeftFor(int loops)
{
    if (loops < 0)
    {
        return -1;
    }
    return std::max(loops, 1) - 1;
}

// Stream from disk or decode up front
ma_uint32 loadFlags(bool stream)
{
    return stream ? MA_RESOURCE_MANAGER_DATA_SOURCE_FLAG_STREAM : MA_RESOURCE_MANAGER_DATA_SOURCE_FLAG_DECODE;
}

// Audio thread read with range and loops
ma_result sourceRead(ma_data_source* dataSource, void* framesOut, ma_uint64 frameCount, ma_uint64* framesRead)
{
    auto* source = static_cast<detail::SoundSource*>(dataSource);
    ma_uint64 total = 0;
    ma_uint64 sinceLoop = 0;
    int emptyLaps = 0;
    ma_result status = MA_SUCCESS;

    while (total < frameCount)
    {
        const ma_uint64 begin = source->beginFrame.load();
        const ma_uint64 end = source->endFrame.load();

        // Start time moved past cursor
        if (source->position < begin)
        {
            if (ma_data_source_seek_to_pcm_frame(&source->file, begin) != MA_SUCCESS)
            {
                break;
            }
            source->position = begin;
        }

        bool finished = source->position >= end;
        if (!finished)
        {
            const ma_uint64 wanted = std::min(frameCount - total, end - source->position);
            // Null output skips frames
            void* destination = framesOut ? static_cast<char*>(framesOut) + total * source->bytesPerFrame : nullptr;
            ma_uint64 got = 0;
            const ma_result result = ma_data_source_read_pcm_frames(&source->file, destination, wanted, &got);

            source->position += got;
            total += got;
            sinceLoop += got;

            // MA_BUSY means stream still loading
            if (result != MA_SUCCESS && result != MA_AT_END)
            {
                status = result;
                break;
            }
            finished = result == MA_AT_END || got == 0;
        }

        if (!finished)
        {
            continue;
        }

        // End of range, stop or loop
        int loops = source->loopsLeft.load();
        if (loops == 0)
        {
            break;
        }
        // Avoid spinning on empty range
        if (sinceLoop == 0 && ++emptyLaps > 1)
        {
            break;
        }
        // Don't overwrite setConfig
        if (loops > 0)
        {
            source->loopsLeft.compare_exchange_strong(loops, loops - 1);
        }
        if (ma_data_source_seek_to_pcm_frame(&source->file, begin) != MA_SUCCESS)
        {
            break;
        }
        source->position = begin;
        sinceLoop = 0;
    }

    if (framesRead)
    {
        *framesRead = total;
    }
    // MA_AT_END only when nothing read
    if (total > 0)
    {
        return MA_SUCCESS;
    }
    return status == MA_SUCCESS ? MA_AT_END : status;
}

// Relative to start time
ma_result sourceSeek(ma_data_source* dataSource, ma_uint64 frameIndex)
{
    auto* source = static_cast<detail::SoundSource*>(dataSource);
    const ma_uint64 target = source->beginFrame.load() + frameIndex;
    const ma_result result = ma_data_source_seek_to_pcm_frame(&source->file, target);
    if (result == MA_SUCCESS)
    {
        source->position = target;
    }
    return result;
}

ma_result sourceFormat(ma_data_source* dataSource, ma_format* format, ma_uint32* channels, ma_uint32* sampleRate, ma_channel* channelMap, size_t channelMapCap)
{
    auto* source = static_cast<detail::SoundSource*>(dataSource);
    return ma_data_source_get_data_format(&source->file, format, channels, sampleRate, channelMap, channelMapCap);
}

ma_result sourceCursor(ma_data_source* dataSource, ma_uint64* cursor)
{
    auto* source = static_cast<detail::SoundSource*>(dataSource);
    const ma_uint64 begin = source->beginFrame.load();
    *cursor = source->position > begin ? source->position - begin : 0;
    return MA_SUCCESS;
}

ma_result sourceLength(ma_data_source* dataSource, ma_uint64* length)
{
    auto* source = static_cast<detail::SoundSource*>(dataSource);
    const ma_uint64 begin = source->beginFrame.load();
    ma_uint64 end = source->endFrame.load();

    if (end == TO_END)
    {
        const ma_result result = ma_data_source_get_length_in_pcm_frames(&source->file, &end);
        if (result != MA_SUCCESS)
        {
            return result;
        }
    }

    *length = end > begin ? end - begin : 0;
    return MA_SUCCESS;
}

// custom functions for ma
ma_data_source_vtable sourceVtable = {
    sourceRead,
    sourceSeek,
    sourceFormat,
    sourceCursor,
    sourceLength,
    nullptr,
    0
};

} // namespace

std::unordered_map<std::string, std::unique_ptr<SoundServer::Asset>> SoundServer::assets;
std::vector<std::unique_ptr<SoundServer::Voice>> SoundServer::voices;
SoundServer::DuplicatePolicy SoundServer::duplicatePolicy = SoundServer::DuplicatePolicy::Print;

SoundServer::Asset::~Asset()
{
    // Engine may already be gone
    if (cached && initialized)
    {
        ma_resource_manager_data_source_uninit(&cache);
    }
}

void SoundServer::Voice::start(const Asset& asset, const SoundConfig& config)
{
    // Same path reuses decoded audio
    ma_resource_manager* resourceManager = ma_engine_get_resource_manager(&engine);
    if (ma_resource_manager_data_source_init(resourceManager, asset.path.c_str(), loadFlags(asset.stream), nullptr, &source.file) != MA_SUCCESS)
    {
        fail("could not open " + asset.path);
    }

    ma_format format;
    ma_uint32 channels = 0;
    ma_uint32 sampleRate = 0;
    if (ma_data_source_get_data_format(&source.file, &format, &channels, &sampleRate, nullptr, 0) != MA_SUCCESS)
    {
        ma_resource_manager_data_source_uninit(&source.file);
        fail("could not read the format of " + asset.path);
    }
    source.sampleRate = sampleRate;
    source.bytesPerFrame = ma_get_bytes_per_frame(format, channels);

    ma_data_source_config sourceConfig = ma_data_source_config_init();
    sourceConfig.vtable = &sourceVtable;
    if (ma_data_source_init(&sourceConfig, &source) != MA_SUCCESS)
    {
        ma_resource_manager_data_source_uninit(&source.file);
        fail("could not create a voice for " + asset.path);
    }

    // Seek to start time
    source.beginFrame = toFrames(config.startTimestamp, sampleRate);
    source.position = 0;
    sourceSeek(&source, 0);

    // 2D only
    if (ma_sound_init_from_data_source(&engine, &source, MA_SOUND_FLAG_NO_SPATIALIZATION, nullptr, &sound) != MA_SUCCESS)
    {
        ma_data_source_uninit(&source);
        ma_resource_manager_data_source_uninit(&source.file);
        fail("could not create a voice for " + asset.path);
    }

    applyConfig(config);
    this->asset = &asset;
    active = true;
    ma_sound_start(&sound);
}

void SoundServer::Voice::release()
{
    if (!active)
    {
        return;
    }
    ma_sound_uninit(&sound);
    ma_data_source_uninit(&source);
    ma_resource_manager_data_source_uninit(&source.file);
    asset = nullptr;
    active = false;
    // 0 is for empty handles
    if (++generation == 0)
    {
        generation = 1;
    }
}

void SoundServer::Voice::restart()
{
    source.loopsLeft = loopsLeftFor(config.loops);
    // Stop first so start() rewinds
    if (ma_sound_at_end(&sound))
    {
        ma_sound_stop(&sound);
    }
    else
    {
        ma_sound_seek_to_pcm_frame(&sound, 0);
    }
    ma_sound_start(&sound);
}

void SoundServer::Voice::applyConfig(const SoundConfig& config)
{
    this->config = config;
    ma_sound_set_volume(&sound, config.volume);
    ma_sound_set_pitch(&sound, config.pitch);
    ma_sound_set_pan(&sound, config.pan);
    source.beginFrame = toFrames(config.startTimestamp, source.sampleRate);
    source.endFrame = config.stopTimestamp < 0.0f ? TO_END : toFrames(config.stopTimestamp, source.sampleRate);
    source.loopsLeft = loopsLeftFor(config.loops);
}

void SoundServer::init()
{
    if (initialized)
    {
        return;
    }
    if (ma_engine_init(nullptr, &engine) != MA_SUCCESS)
    {
        fail("could not start the audio engine");
    }
    initialized = true;
}

void SoundServer::shutdown()
{
    if (!initialized)
    {
        return;
    }
    // Free all before engine stops
    releaseVoices(nullptr);
    voices.clear();
    assets.clear();
    ma_engine_uninit(&engine);
    initialized = false;
}

void SoundServer::update()
{
    if (!initialized)
    {
        return;
    }
    // Free finished voices
    for (auto& voice : voices)
    {
        if (voice->active && ma_sound_at_end(&voice->sound))
        {
            voice->release();
        }
    }
}

void SoundServer::loadSound(const std::string& name, const std::string& path, const SoundConfig& defaultConfig, bool stream)
{
    requireInit();

    auto existing = assets.find(name);
    if (existing != assets.end())
    {
        switch (duplicatePolicy)
        {
            case DuplicatePolicy::Error:
                fail(name + " sound already exists!");

            case DuplicatePolicy::Print:
                std::cerr << ANSI_YELLOW << "[SoundServer] " + name + " sound already exists!" << ANSI_RESET << std::endl;
                return;

            case DuplicatePolicy::Ignore:
                return;

            case DuplicatePolicy::Replace:
                break;

            default:
                fail("invalid duplicate policy!");
        }
    }

    // miniaudio 0.11.25 crashes on failed loads
    std::error_code error;
    if (!std::filesystem::is_regular_file(path, error))
    {
        fail("could not find " + path);
    }

    auto asset = std::make_unique<Asset>();
    asset->path = path;
    asset->defaultConfig = defaultConfig;
    asset->stream = stream;

    ma_resource_manager* resourceManager = ma_engine_get_resource_manager(&engine);
    if (ma_resource_manager_data_source_init(resourceManager, path.c_str(), loadFlags(stream), nullptr, &asset->cache) != MA_SUCCESS)
    {
        fail("could not load " + path);
    }
    // Stream was only a validity check
    if (stream)
    {
        ma_resource_manager_data_source_uninit(&asset->cache);
    }
    else
    {
        asset->cached = true;
    }

    // Replace and stop old copies
    if (existing != assets.end())
    {
        releaseVoices(existing->second.get());
        existing->second = std::move(asset);
        return;
    }
    assets[name] = std::move(asset);
}

void SoundServer::setDefaultConfig(const std::string& name, const SoundConfig& defaultConfig)
{
    auto itr = assets.find(name);
    if (itr == assets.end())
    {
        fail(name + " sound not found!");
    }
    itr->second->defaultConfig = defaultConfig;
}

void SoundServer::removeSound(const std::string& name)
{
    auto itr = assets.find(name);
    if (itr == assets.end())
    {
        fail(name + " sound not found!");
    }
    releaseVoices(itr->second.get());
    assets.erase(itr);
}

void SoundServer::clear()
{
    releaseVoices(nullptr);
    assets.clear();
}

Sound SoundServer::play(const std::string& name)
{
    auto itr = assets.find(name);
    if (itr == assets.end())
    {
        fail(name + " sound not found!");
    }
    return play(name, itr->second->defaultConfig);
}

Sound SoundServer::play(const std::string& name, const SoundConfig& config)
{
    requireInit();

    auto itr = assets.find(name);
    if (itr == assets.end())
    {
        fail(name + " sound not found!");
    }

    // Reuse a free slot
    auto slot = std::find_if(voices.begin(), voices.end(), [](const auto& voice) { return !voice->active; });
    if (slot == voices.end())
    {
        voices.push_back(std::make_unique<Voice>());
        slot = voices.end() - 1;
    }

    Voice& voice = **slot;
    voice.start(*itr->second, config);
    return Sound(static_cast<uint32_t>(slot - voices.begin()), voice.generation);
}

void SoundServer::setMasterVolume(float volume)
{
    requireInit();
    ma_engine_set_volume(&engine, volume);
}

// Null if handle is dead
SoundServer::Voice* SoundServer::getVoice(const Sound& sound)
{
    if (sound.index >= voices.size())
    {
        return nullptr;
    }
    Voice* voice = voices[sound.index].get();
    if (!voice->active || voice->generation != sound.generation)
    {
        return nullptr;
    }
    return voice;
}

// Null releases all
void SoundServer::releaseVoices(const Asset* asset)
{
    for (auto& voice : voices)
    {
        if (voice->active && (asset == nullptr || voice->asset == asset))
        {
            voice->release();
        }
    }
}

} // namespace kerf
