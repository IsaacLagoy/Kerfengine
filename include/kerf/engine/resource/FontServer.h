#pragma once

#include <kerf/engine/resource/Font.h>
#include <memory>
#include <string>
#include <unordered_map>


namespace kerf {

/**
 * @brief Global cache of Font objects, same pattern as TextureServer / ShaderServer.
 *
 * Call loadFont once after the GL context exists, then getFont from any Text node.
 * Owns the unique_ptrs so atlases are freed at program end (or clear()).
 *
 * Not instantiable: everything is static.
 */
class FontServer {
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
    static std::unordered_map<std::string, std::unique_ptr<Font>> fontMap;
    static FontServer::DuplicatePolicy duplicatePolicy;

public:
    FontServer() = delete;
    ~FontServer() = delete;

    /**
     * @brief Bake a .ttf into an atlas and store it under fontName.
     *
     * pixelHeight is raster resolution only. Display size is TextStyle.size
     * and Node::scale.
     */
    static void loadFont(const std::string& fontName, const std::string& fontPath, float pixelHeight);

    static Font* getFont(const std::string& fontName);

    static void removeFont(const std::string& fontName);
    static void clear() { fontMap.clear(); }

    static void setDuplicatePolicy(DuplicatePolicy policy) { duplicatePolicy = policy; }
};

} // namespace kerf