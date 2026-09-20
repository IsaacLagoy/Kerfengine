#pragma once

#include <glad/glad.h>
#include <string>


namespace kerf {

/**
 * @brief OpenGL general purpose texture
 *
 * Empty allocations (width/height constructor) are used as render targets.
 * File-backed textures take their size from the image.
 */
class Texture {
protected:
    GLuint texLoc = 0;

    GLenum internalFormat = GL_RGBA8;
    GLenum format = GL_RGBA;
    GLenum type = GL_UNSIGNED_BYTE;

    int width = 0;
    int height = 0;
    int samples = 1;

    GLenum minFilter = GL_LINEAR;
    GLenum magFilter = GL_LINEAR;

    std::string name;

public:
    /**
     * @brief Allocate an empty GPU texture (framebuffer / render target).
     */
    Texture(int width, int height, GLenum internalFormat, GLenum format, GLenum type=GL_FLOAT, int samples=1);

    /**
     * @brief Load an image from disk. Size and format come from the file.
     *
     * @param textureName key used in TextureServer
     * @param imagePath path relative to the executable, or absolute
     */
    Texture(const std::string& textureName, const std::string& imagePath);
    Texture(const std::string& textureName, const unsigned char* data, int size);

    ~Texture();

    Texture(const Texture&) = delete;
    Texture& operator=(const Texture&) = delete;
    Texture(Texture&&) = delete;
    Texture& operator=(Texture&&) = delete;

    void resize(int width, int height);

    /**
     * @brief Replace GPU storage with pixel data. Resizes to w x h.
     */
    void setPixels(int w, int h, const void* data);

    /**
     * @brief Decode an image and upload it. Replaces size and format.
     */
    void loadFromFile(const std::string& imagePath);
    void loadFromMemory(const unsigned char* data, int size, const char* label);

    GLuint getLoc() const;
    int getWidth() const;
    int getHeight() const;
    const std::string& getName() const;

    void bind() const;
    void unbind() const;
    void destroy();

    /**
     * @brief Sampler filters. Use GL_NEAREST on a low-res render target
     *        if you sample it with a quad instead of blitting.
     */
    void setFilter(GLenum minFilter, GLenum magFilter);

    /**
     * @return GL_TEXTURE_2D_MULTISAMPLE if samples > 1, otherwise GL_TEXTURE_2D
     */
    GLenum target() const;

private:
    void applySamplerState() const;
};

} // namespace kerf