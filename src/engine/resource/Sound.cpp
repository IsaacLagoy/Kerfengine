#include <kerf/engine/resource/Sound.h>

#include "engine/resource/SoundVoice.h"

namespace kerf {

// No-ops once the sound ends

Sound::Sound(uint32_t index, uint32_t generation) :
    index(index),
    generation(generation)
{
}

void Sound::pause()
{
    // Stop keeps the cursor
    if (SoundServer::Voice* voice = SoundServer::getVoice(*this))
    {
        ma_sound_stop(&voice->sound);
    }
}

void Sound::resume()
{
    // Finished sounds get freed
    SoundServer::Voice* voice = SoundServer::getVoice(*this);
    if (voice && !ma_sound_at_end(&voice->sound))
    {
        ma_sound_start(&voice->sound);
    }
}

void Sound::stop()
{
    // Frees the voice now
    if (SoundServer::Voice* voice = SoundServer::getVoice(*this))
    {
        voice->release();
    }
}

void Sound::restart()
{
    if (SoundServer::Voice* voice = SoundServer::getVoice(*this))
    {
        voice->restart();
    }
}

bool Sound::isPlaying() const
{
    // Can briefly read playing after end
    const SoundServer::Voice* voice = SoundServer::getVoice(*this);
    return voice && ma_sound_is_playing(&voice->sound) && !ma_sound_at_end(&voice->sound);
}

SoundConfig Sound::getConfig() const
{
    const SoundServer::Voice* voice = SoundServer::getVoice(*this);
    return voice ? voice->config : SoundConfig{};
}

void Sound::setConfig(const SoundConfig& config)
{
    if (SoundServer::Voice* voice = SoundServer::getVoice(*this))
    {
        voice->applyConfig(config);
    }
}

} // namespace kerf
