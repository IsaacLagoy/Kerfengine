#pragma once

#include <glad/glad.h>
#include <optional>


/**
 * @brief OpenGL general purpose texture
 * 
 */
class Texture {
protected:
    // location of the texture
    GLuint texLoc;

    // the internal format of the buffer
    GLenum internalFormat;

    // the external format of the buffer
    GLenum format;

    // data type used in the buffer. ex GL_FLOAT
    GLenum type;

    // width of the buffer
    int width;

    // height of the buffer
    int height;

    // MSAA sample count
    int samples;

public:
    /**
     * @brief Construct a new Texture object
     * 
     * @param width width of the texture
     * @param height height of the texture
     * @param internalFormat internal format of the data
     * @param format external format of the data
     * @param type data type used to create the buffer
     * @param samples number of MSAA samples, use 1 for no multisampling
     */
    Texture(int width, int height, GLenum internalFormat, GLenum format, GLenum type=GL_FLOAT, int samples=1);

    /**
     * @brief Destroy the Texture object
     * 
     */
    ~Texture();

    // avoid copying to prevent the texture from destroying itself
    Texture(const Texture&) = delete;
    Texture& operator=(const Texture&) = delete;

    /**
     * @brief Construct a new Texture object
     * 
     * @param other the Texture object to move
     */
    Texture(Texture&& other) noexcept;

    /**
     * @brief Moves a Texture to this Texture
     * 
     * @param other the Texture object to move
     * @return Texture& 
     */
    Texture& operator=(Texture&& other) noexcept;

    /**
     * @brief Rebuilds the texture with the new dimensions
     * 
     * @param width new texture width
     * @param height new texture height
     */
    void resize(int width, int height);

    /**
     * @brief Get the Loc object
     * 
     * @return GLuint 
     */
    GLuint getLoc() const;

    /**
     * @brief shorthand for glBindTexture(..., texLoc)
     * 
     */
    void bind() const;

    /**
     * @brief shorthand for glBindTexture(..., 0)
     * 
     */
    void unbind() const;

    /**
     * @brief Deletes the OpenGL bindings for the texture
     * 
     */
    void destroy();

    /**
     * @brief Gives the texture target for the current samples
     * 
     * Returns GL_TEXTURE_2D_MULTISAMPLE if samples > 1
     * Otherwise GL_TEXTURE_2D
     * 
     * @return GLenum 
     */
    GLenum target() const;
};