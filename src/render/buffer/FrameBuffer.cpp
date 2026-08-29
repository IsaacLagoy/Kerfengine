#include "FrameBuffer.h"

#include "render/texture/Texture.h"
#include "shared/Const.h"

#include <stdexcept>


FrameBuffer::FrameBuffer() :
    fbo(0),
    texture(nullptr)
{}

FrameBuffer::~FrameBuffer()
{
    if (fbo) glDeleteFramebuffers(1, &fbo);
}

void FrameBuffer::setTexture(Texture* texture, int width, int height)
{
    this->width = width;
    this->height = height;
    this->texture = texture;

    // create the fbo if this is our first bind
    if (!fbo) 
    {
        glGenFramebuffers(1, &fbo);
    }

    // attach texture
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture->getLoc(), 0);

    // validation
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        throw std::runtime_error(ANSI_RED + "[Gbuffer] framebuffer incomplete!" + ANSI_RESET);
    }

    unbind();
}

GLuint FrameBuffer::getFBO() const
{
    return fbo;
}

void FrameBuffer::bind()
{
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glViewport(0, 0, width, height);
}

void FrameBuffer::unbind()
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

