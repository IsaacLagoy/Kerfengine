#include "engine/resource/TextureServer.h"

#include "shared/Const.h"

#include <stdexcept>
#include <iostream>


std::unordered_map<std::string, std::unique_ptr<Texture>> TextureServer::textureMap;
TextureServer::DuplicatePolicy TextureServer::duplicatePolicy = TextureServer::DuplicatePolicy::Print;

void TextureServer::loadTexture(const std::string& textureName, const std::string& imagePath)
{
    if (textureMap.find(textureName) == textureMap.end())
    {
        textureMap[textureName] = std::make_unique<Texture>(textureName, imagePath);
        return;
    }

    switch (duplicatePolicy)
    {
        case TextureServer::DuplicatePolicy::Error:
            throw std::runtime_error(ANSI_RED + "[TextureServer] " + textureName + " texture already exists!" + ANSI_RESET);

        case TextureServer::DuplicatePolicy::Print:
            std::cerr << ANSI_YELLOW << "[TextureServer] " + textureName + " texture already exists!" << ANSI_RESET << std::endl;
            return;

        case TextureServer::DuplicatePolicy::Ignore:
            return;

        case TextureServer::DuplicatePolicy::Replace:
        {
            auto replacement = std::make_unique<Texture>(textureName, imagePath);
            textureMap[textureName] = std::move(replacement);
            return;
        }

        default:
            throw std::runtime_error(ANSI_RED + "[TextureServer] invalid duplicate policy!" + ANSI_RESET);
    }
}

Texture* TextureServer::getTexture(const std::string& textureName)
{
    auto itr = textureMap.find(textureName);
    if (itr == textureMap.end())
    {
        throw std::runtime_error(ANSI_RED + "[TextureServer] " + textureName + " texture not found!" + ANSI_RESET);
    }
    return itr->second.get();
}

void TextureServer::removeTexture(const std::string& textureName)
{
    auto itr = textureMap.find(textureName);
    if (itr == textureMap.end())
    {
        throw std::runtime_error(ANSI_RED + "[TextureServer] " + textureName + " texture not found!" + ANSI_RESET);
    }
    textureMap.erase(itr);
}
