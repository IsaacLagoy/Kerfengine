#pragma once

#include <kerf/engine/resource/Sound.h>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace kerf {

class SoundServer {
public:
    enum class DuplicatePolicy {
        Invalid = 0,
        Error,
        Print,
        Ignore,
        Replace,
        Count
    };

private:
    struct Asset;
    struct Voice;

    static std::unordered_map<std::string, std::unique_ptr<Asset>> assets;
    static std::vector<std::unique_ptr<Voice>> voices;
    static DuplicatePolicy duplicatePolicy;

    static Voice* getVoice(const Sound& sound);
    static void releaseVoices(const Asset* asset);

    friend class Sound;

public:
    SoundServer() = delete;
    ~SoundServer() = delete;

    static void init();
    static void shutdown();
    static void update();

    static void loadSound(const std::string& name, const std::string& path, const SoundConfig& defaultConfig = {}, bool stream = false);
    static void setDefaultConfig(const std::string& name, const SoundConfig& defaultConfig);
    static void removeSound(const std::string& name);
    static void clear();

    static Sound play(const std::string& name);
    static Sound play(const std::string& name, const SoundConfig& config);

    static void setMasterVolume(float volume);
    static void setDuplicatePolicy(DuplicatePolicy policy) { duplicatePolicy = policy; }
};

} // namespace kerf
