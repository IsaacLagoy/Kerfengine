#include <kerf/engine/resource/Texture.h>
#include <stdexcept>

#include <kerf/shared/Const.h>
#include <kerf/shared/Files.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>


namespace kerf {

static void formatsFromChannelCount(int channels, GLenum& internalFormat, GLenum& format)
{
    switch (channels)
    {
        case 1:
            internalFormat = GL_R8;
            format = GL_RED;
            return;
        case 2:
            internalFormat = GL_RG8;
            format = GL_RG;
            return;
        case 3:
            internalFormat = GL_RGB8;
            format = GL_RGB;
            return;
        case 4:
            internalFormat = GL_RGBA8;
            format = GL_RGBA;
            return;
        default:
            throw std::runtime_error(ANSI_RED + "[Texture] unsupported channel count " + std::to_string(channels) + ANSI_RESET);
    }
}


Texture::Texture(int width, int height, GLenum internalFormat, GLenum format, GLenum type, int samples) :
    texLoc(0),
    internalFormat(internalFormat),
    format(format),
    type(type),
    width(width),
    height(height),
    samples(samples)
{
    glGenTextures(1, &texLoc);
    resize(width, height);
}

Texture::Texture(const std::string& textureName, const std::string& imagePath) :
    texLoc(0),
    type(GL_UNSIGNED_BYTE),
    samples(1),
    name(textureName)
{
    glGenTextures(1, &texLoc);
    loadFromFile(imagePath);
}

Texture::Texture(const std::string& textureName, const unsigned char* data, int size) :
    texLoc(0),
    type(GL_UNSIGNED_BYTE),
    samples(1),
    name(textureName)
{
    glGenTextures(1, &texLoc);
    loadFromMemory(data, size, textureName.c_str());
}

Texture::~Texture()
{
    destroy();
}

void Texture::applySamplerState() const
{
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magFilter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    if (format == GL_DEPTH_COMPONENT)
    {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_NONE);
    }
}

void Texture::setFilter(GLenum minFilter, GLenum magFilter)
{
    this->minFilter = minFilter;
    this->magFilter = magFilter;
    if (samples != 1) return;

    bind();
    applySamplerState();
    unbind();
}

void Texture::resize(int width, int height)
{
    this->width = width;
    this->height = height;

    bind();

    if (samples == 1)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, type, nullptr);
        applySamplerState();
    }
    else
    {
        glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, samples, internalFormat, width, height, GL_TRUE);
    }

    unbind();
}

void Texture::setPixels(int w, int h, const void* data)
{
    if (samples != 1)
    {
        throw std::runtime_error(ANSI_RED + "[Texture] cannot upload pixels to a multisampled texture" + ANSI_RESET);
    }

    width = w;
    height = h;

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    bind();
    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, type, data);
    applySamplerState();
    unbind();
}

void Texture::loadFromFile(const std::string& imagePath)
{
    const std::string resolved = Files::resolvePath(imagePath);
    if (samples != 1)
    {
        throw std::runtime_error(ANSI_RED + "[Texture] cannot load an image into a multisampled texture" + ANSI_RESET);
    }

    stbi_set_flip_vertically_on_load(1);

    int channels = 0;
    unsigned char* pixels = stbi_load(resolved.c_str(), &width, &height, &channels, 0);
    if (!pixels)
    {
        const char* reason = stbi_failure_reason();
        throw std::runtime_error(
            ANSI_RED + "[Texture] failed to load " + resolved + ": " +
            (reason ? reason : "unknown error") + ANSI_RESET
        );
    }

    formatsFromChannelCount(channels, internalFormat, format);
    type = GL_UNSIGNED_BYTE;
    setPixels(width, height, pixels);
    stbi_image_free(pixels);
}

void Texture::loadFromMemory(const unsigned char* data, int size, const char* label)
{
    if (samples != 1)
    {
        throw std::runtime_error(ANSI_RED + "[Texture] cannot load an image into a multisampled texture" + ANSI_RESET);
    }

    stbi_set_flip_vertically_on_load(1);

    int channels = 0;
    unsigned char* pixels = stbi_load_from_memory(data, size, &width, &height, &channels, 0);
    if (!pixels)
    {
        const char* reason = stbi_failure_reason();
        throw std::runtime_error(
            ANSI_RED + "[Texture] failed to load " + label + ": " +
            (reason ? reason : "unknown error") + ANSI_RESET
        );
    }

    formatsFromChannelCount(channels, internalFormat, format);
    type = GL_UNSIGNED_BYTE;
    setPixels(width, height, pixels);
    stbi_image_free(pixels);
}

void Texture::bind() const
{
    glBindTexture(target(), texLoc);
}

void Texture::unbind() const
{
    glBindTexture(target(), 0);
}

GLuint Texture::getLoc() const
{
    return texLoc;
}

int Texture::getWidth() const
{
    return width;
}

int Texture::getHeight() const
{
    return height;
}

const std::string& Texture::getName() const
{
    return name;
}

void Texture::destroy()
{
    if (texLoc)
    {
        glDeleteTextures(1, &texLoc);
        texLoc = 0;
    }
}

GLenum Texture::target() const
{
    return samples > 1 ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D;
}

} // namespace kerf