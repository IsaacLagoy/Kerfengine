#include <kerf/engine/render/buffer/FrameBuffer.h>

#include <kerf/engine/resource/Texture.h>
#include <kerf/shared/Const.h>

#include <stdexcept>

namespace kerf {

FrameBuffer::FrameBuffer() :
    fbo(0),
    depthRbo(0),
    texture(nullptr)
{}

FrameBuffer::~FrameBuffer()
{
    if (depthRbo) glDeleteRenderbuffers(1, &depthRbo);
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

    if (!depthRbo)
    {
        glGenRenderbuffers(1, &depthRbo);
    }

    // attach texture
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture->getLoc(), 0);

    // attach depth
    glBindRenderbuffer(GL_RENDERBUFFER, depthRbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, depthRbo);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);

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

int FrameBuffer::getWidth() const
{
    return width;
}

int FrameBuffer::getHeight() const
{
    return height;
}

Texture* FrameBuffer::getTexture() const
{
    return texture;
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

} // namespace kerf