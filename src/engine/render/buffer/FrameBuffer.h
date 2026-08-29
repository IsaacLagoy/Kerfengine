#pragma once

#include <glad/glad.h>


class Texture;


/**
 * @brief Reusable framebuffer for rendering to textures
 * 
 */
class FrameBuffer {
private:
    // location of the fbo 
    GLuint fbo = 0;

    // texture to render to
    Texture* texture = nullptr; // does not own

    // texture width
    int width;

    // texture height
    int height;

public:
    /**
     * @brief Construct a new Frame Buffer object
     * 
     */
    FrameBuffer();

    /**
     * @brief Destroy the Frame Buffer object
     * 
     */
    ~FrameBuffer();

    /**
     * @brief Binds texture to the framebuffer
     * 
     * @param texture render target
     * @param width width of texture in pixels
     * @param height height of texture in pixels
     */
    void setTexture(Texture* texture, int width, int height);

    /**
     * @brief Gets FBO location
     * 
     * @return GLuint 
     */
    GLuint getFBO() const;

    /**
     * @brief Binds the FBO and sets the viewport to match
     * 
     */
    void bind();

    /**
     * @brief Unbinds the FBO
     * 
     */
    void unbind();

};