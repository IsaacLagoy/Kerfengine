#include <kerf/engine/render/buffer/FrameBuffer.h>

#include <kerf/engine/resource/Texture.h>
#include <kerf/shared/Const.h>

#include <stdexcept>
#include <vector>

namespace kerf {

FrameBuffer::FrameBuffer() :
    fbo(0),
    depthRbo(0)
{}

FrameBuffer::~FrameBuffer()
{
    if (depthRbo) glDeleteRenderbuffers(1, &depthRbo);
    if (fbo) glDeleteFramebuffers(1, &fbo);
}

void FrameBuffer::setAttachments(const std::vector<Attachment>& attachments)
{
    if (attachments.empty())
    {
        throw std::runtime_error(ANSI_RED + "[Gbuffer] setAttachments requires at least one texture" + ANSI_RESET);
    }

    Texture* firstColor = nullptr;
    Texture* depthTexture = nullptr;
    std::vector<Texture*> colorTextures;
    colorTextures.reserve(attachments.size());

    for (const Attachment& attachment : attachments)
    {
        if (!attachment.texture)
        {
            throw std::runtime_error(ANSI_RED + "[Gbuffer] attachment '" + attachment.name + "' is null" + ANSI_RESET);
        }

        if (attachment.format == Format::Depth24)
        {
            if (depthTexture)
            {
                throw std::runtime_error(ANSI_RED + "[Gbuffer] duplicate depth attachment" + ANSI_RESET);
            }
            depthTexture = attachment.texture;
            continue;
        }

        if (!firstColor) firstColor = attachment.texture;
        colorTextures.push_back(attachment.texture);
    }

    Texture* sizeSource = firstColor ? firstColor : depthTexture;
    width = sizeSource->getWidth();
    height = sizeSource->getHeight();

    if (!fbo)
    {
        glGenFramebuffers(1, &fbo);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    for (int i = 0; i < colorAttachmentCount; ++i)
    {
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, 0, 0);
    }

    std::vector<GLenum> drawBuffers;
    drawBuffers.reserve(colorTextures.size());
    for (size_t i = 0; i < colorTextures.size(); ++i)
    {
        glFramebufferTexture2D(
            GL_FRAMEBUFFER,
            GL_COLOR_ATTACHMENT0 + static_cast<GLenum>(i),
            GL_TEXTURE_2D,
            colorTextures[i]->getLoc(),
            0
        );
        drawBuffers.push_back(GL_COLOR_ATTACHMENT0 + static_cast<GLenum>(i));
    }
    colorAttachmentCount = static_cast<int>(colorTextures.size());

    if (drawBuffers.empty())
    {
        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);
    }
    else
    {
        glDrawBuffers(static_cast<GLsizei>(drawBuffers.size()), drawBuffers.data());
    }

    if (depthTexture)
    {
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, 0);
        glFramebufferTexture2D(
            GL_FRAMEBUFFER,
            GL_DEPTH_ATTACHMENT,
            GL_TEXTURE_2D,
            depthTexture->getLoc(),
            0
        );
    }
    else
    {
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, 0, 0);
        if (!depthRbo)
        {
            glGenRenderbuffers(1, &depthRbo);
        }
        glBindRenderbuffer(GL_RENDERBUFFER, depthRbo);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, depthRbo);
        glBindRenderbuffer(GL_RENDERBUFFER, 0);
    }

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
