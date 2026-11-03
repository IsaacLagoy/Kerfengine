#pragma once

#include <cstdint>

namespace kerf {

struct SoundConfig {
    float volume = 1.0f;
    float pitch = 1.0f;
    float pan = 0.0f;
    float startTimestamp = 0.0f;
    float stopTimestamp = -1.0f;
    int loops = 1;
};

class Sound {
private:
    uint32_t index = 0;
    uint32_t generation = 0;

    Sound(uint32_t index, uint32_t generation);

    friend class SoundServer;

public:
    Sound() = default;

    void pause();
    void resume();
    void stop();
    void restart();
    bool isPlaying() const;

    SoundConfig getConfig() const;
    void setConfig(const SoundConfig& config);
};

} // namespace kerf
