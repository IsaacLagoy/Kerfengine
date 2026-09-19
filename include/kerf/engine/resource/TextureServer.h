#pragma once

#include <kerf/engine/resource/Texture.h>
#include <unordered_map>
#include <memory>
#include <string>


namespace kerf {

/**
 * @brief Global class used to load and service file-backed textures.
 *
 * Stores textures as unique_ptrs and cleans them up at program end.
 * Empty render-target textures should be constructed directly, not through this server.
 */
class TextureServer {
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
    static std::unordered_map<std::string, std::unique_ptr<Texture>> textureMap;
    static TextureServer::DuplicatePolicy duplicatePolicy;

public:
    TextureServer() = delete;
    ~TextureServer() = delete;

    /**
     * @brief Load an image into the server.
     *
     * @param textureName key for accessing the texture
     * @param imagePath path relative to the executable, or absolute
     */
    static void loadTexture(const std::string& textureName, const std::string& imagePath);

    static Texture* getTexture(const std::string& textureName);

    static void removeTexture(const std::string& textureName);
    static void clear() { textureMap.clear(); }

    static void setDuplicatePolicy(DuplicatePolicy policy) { duplicatePolicy = policy; }
};

} // namespace kerf