#pragma once

#include <kerf/engine/resource/SoundServer.h>
#include <miniaudio.h>
#include <atomic>
#include <string>
#include <type_traits>

namespace kerf {

namespace detail {

// File wrapper that handles range and loops
struct SoundSource {
    ma_data_source_base base{};
    ma_resource_manager_data_source file{};
    ma_uint32 sampleRate = 0;
    ma_uint32 bytesPerFrame = 0;
    ma_uint64 position = 0;                  // audio thread only
    std::atomic<ma_uint64> beginFrame{0};
    std::atomic<ma_uint64> endFrame{0};      // ~0 is end of file
    std::atomic<int> loopsLeft{0};           // -1 is forever
};

// base must stay first
static_assert(std::is_standard_layout_v<SoundSource>);

} // namespace detail

// A loaded sound file
struct SoundServer::Asset {
    std::string path;
    SoundConfig defaultConfig;
    bool stream = false;
    bool cached = false;
    ma_resource_manager_data_source cache{};  // shared decoded audio

    ~Asset();
};

// One playback of an asset
struct SoundServer::Voice {
    detail::SoundSource source;
    ma_sound sound{};
    SoundConfig config;
    const Asset* asset = nullptr;
    uint32_t generation = 1;  // invalidates old handles
    bool active = false;

    void start(const Asset& asset, const SoundConfig& config);
    void release();
    void restart();
    void applyConfig(const SoundConfig& config);
};

} // namespace kerf
