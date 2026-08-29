#include "Texture.h"

#include <utility>


Texture::Texture(int width, int height, GLenum internalFormat, GLenum format, GLenum type, int samples) :
    texLoc(0),
    internalFormat(internalFormat),
    format(format),
    type(type),
    width(width),
    height(height),
    samples(samples)
{
    // create texture location in GL and build the new texture with resize
    glGenTextures(1, &texLoc);
    resize(width, height);
}

Texture::~Texture()
{
    destroy();
}

Texture::Texture(Texture&& other) noexcept : 
    texLoc(std::exchange(other.texLoc, 0)),
    internalFormat(other.internalFormat),
    format(other.format),
    type(other.type),
    width(other.width),
    height(other.height),
    samples(other.samples)
{
    // excahnge the texture locations so the moved texture does not delete the location
}

Texture& Texture::operator=(Texture&& other) noexcept
{
    if (this != &other)
    {
        // clear ourselves and transfer all data
        destroy();
        texLoc         = std::exchange(other.texLoc, 0);
        internalFormat = other.internalFormat;
        format         = other.format;
        type           = other.type;
        width          = other.width;
        height         = other.height;
        samples        = other.samples;
    }
    return *this;
}

void Texture::resize(int width, int height)
{
    this->width = width;
    this->height = height;

    // bind texture so GL modifies our location
    // standard OpenGL state kludge
    bind();

    if (samples == 1)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, type, nullptr);

        // these properties are not available for multisampled textures
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    }
    else
    {
        glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, samples, internalFormat, width, height, GL_TRUE);
    }

    unbind();
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
    return this->texLoc;
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