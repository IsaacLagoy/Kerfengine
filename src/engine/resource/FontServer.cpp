#include <kerf/engine/resource/FontServer.h>
#include <iostream>
#include <stdexcept>

#include <kerf/shared/Const.h>


namespace kerf {
    
std::unordered_map<std::string, std::unique_ptr<Font>> FontServer::fontMap;
FontServer::DuplicatePolicy FontServer::duplicatePolicy = FontServer::DuplicatePolicy::Print;

void FontServer::loadFont(const std::string& fontName, const std::string& fontPath, float pixelHeight)
{
    if (fontMap.find(fontName) == fontMap.end())
    {
        fontMap[fontName] = std::make_unique<Font>(fontName, fontPath, pixelHeight);
        return;
    }

    // Key already loaded: honor the same duplicate policy as TextureServer.
    switch (duplicatePolicy)
    {
        case FontServer::DuplicatePolicy::Error:
            throw std::runtime_error(ANSI_RED + "[FontServer] " + fontName + " font already exists!" + ANSI_RESET);

        case FontServer::DuplicatePolicy::Print:
            std::cerr << ANSI_YELLOW << "[FontServer] " + fontName + " font already exists!" << ANSI_RESET << std::endl;
            return;

        case FontServer::DuplicatePolicy::Ignore:
            return;

        case FontServer::DuplicatePolicy::Replace:
            fontMap[fontName] = std::make_unique<Font>(fontName, fontPath, pixelHeight);
            return;

        default:
            throw std::runtime_error(ANSI_RED + "[FontServer] invalid duplicate policy!" + ANSI_RESET);
    }
}

Font* FontServer::getFont(const std::string& fontName)
{
    auto itr = fontMap.find(fontName);
    if (itr == fontMap.end())
    {
        throw std::runtime_error(ANSI_RED + "[FontServer] " + fontName + " font not found!" + ANSI_RESET);
    }
    return itr->second.get();
}

void FontServer::removeFont(const std::string& fontName)
{
    auto itr = fontMap.find(fontName);
    if (itr == fontMap.end())
    {
        throw std::runtime_error(ANSI_RED + "[FontServer] " + fontName + " font not found!" + ANSI_RESET);
    }
    fontMap.erase(itr);
}

} // namespace kerf