#include <kerf/engine/render/pipeline/Target.h>

#include <kerf/engine/render/buffer/FrameBuffer.h>
#include <kerf/engine/resource/ShaderServer.h>
#include <kerf/engine/resource/Texture.h>
#include <kerf/shared/Const.h>

#include <glad/glad.h>
#include <algorithm>
#include <stdexcept>
#include <unordered_set>
#include <vector>


namespace kerf {

namespace {

void glFormatsFor(Format format, GLenum& internalFormat, GLenum& formatEnum, GLenum& type)
{
    switch (format)
    {
        case Format::RGBA8:
        {
            internalFormat = GL_RGBA8;
            formatEnum = GL_RGBA;
            type = GL_UNSIGNED_BYTE;
            return;
        }
        case Format::RGB16F:
        {
            internalFormat = GL_RGBA16F;
            formatEnum = GL_RGBA;
            type = GL_FLOAT;
            return;
        }
        case Format::Depth24:
        {
            internalFormat = GL_DEPTH_COMPONENT24;
            formatEnum = GL_DEPTH_COMPONENT;
            type = GL_FLOAT;
            return;
        }
        case Format::UnsignedInt8:
        {
            internalFormat = GL_R8UI;
            formatEnum = GL_RED_INTEGER;
            type = GL_UNSIGNED_BYTE;
            return;
        }
    }
    
    throw std::runtime_error(ANSI_RED + "[Target] unsupported format" + ANSI_RESET);
}

void validateSize(const TargetSize& size)
{
    if (size.mode == TargetSize::Mode::Scale)
    {
        if (size.scale <= 0.0f)
        {
            throw std::runtime_error(ANSI_RED + "[Target] scale must be positive" + ANSI_RESET);
        }
        return;
    }

    if (size.width <= 0)
    {
        throw std::runtime_error(ANSI_RED + "[Target] pixel width must be positive" + ANSI_RESET);
    }
}

glm::ivec2 resolvePixels(const TargetSize& size, int framebufferWidth, int framebufferHeight)
{
    const int fbW = std::max(framebufferWidth, 1);
    const int fbH = std::max(framebufferHeight, 1);

    // scale size
    if (size.mode == TargetSize::Mode::Scale)
    {
        return glm::ivec2(
            std::max(1, static_cast<int>(fbW * size.scale)),
            std::max(1, static_cast<int>(fbH * size.scale))
        );
    }

    // pixel size
    const int width = size.width;
    int height = size.height;
    if (height <= 0)
    {
        // scale height to fit aspect ratio TODO we may want to add Letterboy
        height = std::max(1, static_cast<int>(
            (static_cast<long long>(width) * fbH + fbW / 2) / fbW
        ));
    }
    return glm::ivec2(width, height);
}

GLenum filterToGL(Filter filter)
{
    switch (filter)
    {
        case Filter::Nearest:
            return GL_NEAREST;
        case Filter::Linear:
            return GL_LINEAR;
    }
    throw std::runtime_error(ANSI_RED + "[Target] unsupported filter" + ANSI_RESET);
}

void require(const Target& target)
{
    if (!target)
    {
        throw std::runtime_error(ANSI_RED + "[Target] invalid target" + ANSI_RESET);
    }
}

} // namespace

// ------------------------------------------------------------
// TargetPlane
// ------------------------------------------------------------

TargetPlane::TargetPlane(std::string name, Format format) : 
    name(std::move(name)), 
    format(format), 
    clear(glm::vec4(0.0f)), 
    clearBeforeRender(true)
{}

TargetPlane::TargetPlane(std::string name, Format format, const glm::vec4& clear, bool clearBeforeRender) : 
    name(std::move(name)), 
    format(format), 
    clear(clear), 
    clearBeforeRender(clearBeforeRender)
{}

const std::string& TargetPlane::getName() const
{
    return name;
}

Format TargetPlane::getFormat() const
{
    return format;
}

const glm::vec4& TargetPlane::getClear() const
{
    return clear;
}

bool TargetPlane::getClearBeforeRender() const
{
    return clearBeforeRender;
}

Texture* TargetPlane::getTexture() const
{
    return texture.get();
}

int TargetPlane::getColorIndex() const
{
    return colorIndex;
}

void TargetPlane::setClear(const glm::vec4& clear)
{
    this->clear = clear;
}

void TargetPlane::setColorIndex(int colorIndex)
{
    this->colorIndex = colorIndex;
}

void TargetPlane::setTexture(std::unique_ptr<Texture> texture)
{
    this->texture = std::move(texture);
}

// ------------------------------------------------------------
// TargetImpl
// ------------------------------------------------------------

TargetPlane* TargetImpl::findPlane(const std::string& name)
{
    for (TargetPlane& plane : planes)
    {
        if (plane.getName() == name) return &plane;
    }
    return nullptr;
}

Texture* TargetImpl::soleColorTexture() const
{
    Texture* color = nullptr;

    // present only supports a single color attachment (depth is ignored).
    for (const TargetPlane& plane : planes)
    {
        if (plane.getFormat() == Format::Depth24) 
        {
            continue;
        }

        // already has a color but another is found
        if (color)
        {
            throw std::runtime_error(ANSI_RED + "[Target] present requires exactly one color plane" + ANSI_RESET);
        }
        color = plane.getTexture();
    }

    if (!color)
    {
        throw std::runtime_error(ANSI_RED + "[Target] present requires exactly one color plane" + ANSI_RESET);
    }
    return color;
}

void TargetImpl::clearBound(const TargetPlane& plane) const
{
    glm::vec4 clear = plane.getClear();

    // depth clear value is stored in the red channel.
    if (plane.getFormat() == Format::Depth24)
    {
        const GLfloat depth = clear.r;
        glClearBufferfv(GL_DEPTH, 0, &depth);
        return;
    }

    // add 0.5 to the clear value to avoid precision issues
    if (plane.getFormat() == Format::UnsignedInt8)
    {
        const GLuint value[4] = {
            static_cast<GLuint>(clear.r + 0.5f),
            static_cast<GLuint>(clear.g + 0.5f),
            static_cast<GLuint>(clear.b + 0.5f),
            static_cast<GLuint>(clear.a + 0.5f)
        };
        glClearBufferuiv(GL_COLOR, plane.getColorIndex(), value);
        return;
    }

    // Default float/RGBA color clear.
    const GLfloat value[4] = { clear.r, clear.g, clear.b, clear.a };
    glClearBufferfv(GL_COLOR, plane.getColorIndex(), value);
}

void TargetImpl::allocate(int w, int h)
{
    width = w;
    height = h;

    int colorIndex = 0;
    std::vector<FrameBuffer::Attachment> attachments;
    attachments.reserve(planes.size());

    // create frame buffer attachments
    for (TargetPlane& plane : planes)
    {
        GLenum internalFormat = GL_RGBA8;
        GLenum formatEnum = GL_RGBA;
        GLenum type = GL_UNSIGNED_BYTE;
        glFormatsFor(plane.getFormat(), internalFormat, formatEnum, type);

        // depth stays nearest, color planes use the target sampling filter
        const GLenum glFilter = plane.getFormat() == Format::Depth24 ? GL_NEAREST : filterToGL(filter);
        if (!plane.getTexture())
        {
            plane.setTexture(std::make_unique<Texture>(width, height, internalFormat, formatEnum, type));
        }
        else
        {
            plane.getTexture()->resize(width, height);
        }
        plane.getTexture()->setFilter(glFilter, glFilter);

        // assign FBO color attachment indices (depth is not a color attachment)
        if (plane.getFormat() == Format::Depth24)
        {
            plane.setColorIndex(-1);
        }
        else
        {
            plane.setColorIndex(colorIndex++);
        }

        attachments.push_back(FrameBuffer::Attachment{
            plane.getName(),
            plane.getTexture(),
            plane.getFormat()
        });
    }

    // lazily create the framebuffer, then refresh all attachments
    if (!fbo)
    {
        fbo = std::make_unique<FrameBuffer>();
    }
    fbo->setAttachments(attachments);
}

// ------------------------------------------------------------
// Target
// ------------------------------------------------------------

Target::Target(TargetSize size, Filter filter, std::initializer_list<TargetPlane> planes) : impl(std::make_shared<TargetImpl>())
{
    if (planes.size() == 0)
    {
        throw std::runtime_error(ANSI_RED + "[Target] at least one plane is required" + ANSI_RESET);
    }
    validateSize(size);

    // copy planes into impl after validating names and depth rules
    std::unordered_set<std::string> names;
    bool haveDepth = false;
    impl->planes.reserve(planes.size());
    for (const TargetPlane& plane : planes)
    {
        if (plane.getName().empty())
        {
            throw std::runtime_error(ANSI_RED + "[Target] name must not be empty" + ANSI_RESET);
        }

        if (!names.insert(plane.getName()).second)
        {
            throw std::runtime_error(
                ANSI_RED + "[Target] duplicate plane name '" + plane.getName() + "'" + ANSI_RESET
            );
        }

        // default depth clear to far plane (1.0) when unset
        glm::vec4 planeClear = plane.getClear();
        if (plane.getFormat() == Format::Depth24)
        {
            if (haveDepth)
            {
                throw std::runtime_error(ANSI_RED + "[Target] at most one depth plane is allowed" + ANSI_RESET);
            }
            haveDepth = true;
            if (planeClear == glm::vec4(0.0f))
            {
                planeClear = glm::vec4(1.0f, 0.0f, 0.0f, 0.0f);
            }
        }

        TargetPlane stored(
            plane.getName(), 
            plane.getFormat(), 
            planeClear, 
            plane.getClearBeforeRender()
        );
        impl->planes.push_back(std::move(stored));
    }

    impl->size = size;
    impl->filter = filter;
    impl->allocate(1, 1);
}

Target::Target(TargetSize size, Format format, Filter filter, std::string name) : Target(
    size, 
    filter, 
    { TargetPlane{ std::move(name), format } }
) {}

Filter Target::filter() const
{
    require(*this);
    return impl->filter;
}

int Target::width() const
{
    return impl ? impl->width : 0;
}

int Target::height() const
{
    return impl ? impl->height : 0;
}

bool Target::ensureSize(int framebufferWidth, int framebufferHeight) const
{
    require(*this);

    // reallocate textures when scale or pixel size resolves differently
    const glm::ivec2 pixels = resolvePixels(impl->size, framebufferWidth, framebufferHeight);
    if (pixels.x == impl->width && pixels.y == impl->height)
    {
        return false;
    }
    impl->allocate(pixels.x, pixels.y);
    return true;
}

void Target::beginOutput() const
{
    require(*this);

    // bind offscreen target and clear any plane marked clearBeforeRender
    impl->fbo->bind();
    for (const TargetPlane& plane : impl->planes)
    {
        if (plane.getClearBeforeRender())
        {
            impl->clearBound(plane);
        }
    }
}

void Target::endOutput() const
{
    require(*this);
    
    // return to the default framebuffer
    impl->fbo->unbind();
}

void Target::clearPlane(const std::string& name) const
{
    require(*this);
    TargetPlane* plane = impl->findPlane(name);
    if (!plane)
    {
        throw std::runtime_error(
            ANSI_RED + "[Target] no plane named '" + name + "'" + ANSI_RESET
        );
    }

    // bind this target's FBO only if it is not already active
    GLint previous = 0;
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &previous);
    const GLuint fbo = impl->fbo->getFBO();
    const bool rebind = static_cast<GLuint>(previous) != fbo;
    if (rebind)
    {
        impl->fbo->bind();
    }

    impl->clearBound(*plane);
    if (rebind)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, static_cast<GLuint>(previous));
    }
}

void Target::bindSamplers(Shader* shader, std::span<const Target> inputs)
{
    if (!shader)
    {
        throw std::runtime_error(ANSI_RED + "[Target] bindSamplers has a null shader" + ANSI_RESET);
    }

    std::unordered_set<Texture*> textures;
    std::unordered_set<std::string> names;
    int unit = 0;
    Texture* texelSource = nullptr;

    // bind each input plane to a texture unit; uniform name matches plane name
    for (const Target& input : inputs)
    {
        require(input);
        for (const TargetPlane& plane : input.impl->planes)
        {
            Texture* texture = plane.getTexture();
            if (!texture)
            {
                throw std::runtime_error(ANSI_RED + "[Target] null plane '" + plane.getName() + "'" + ANSI_RESET);
            }
            if (!textures.insert(texture).second)
            {
                throw std::runtime_error(ANSI_RED + "[Target] duplicate input plane '" + plane.getName() + "'" + ANSI_RESET);
            }
            if (!names.insert(plane.getName()).second)
            {
                throw std::runtime_error(ANSI_RED + "[Target] duplicate input name '" + plane.getName() + "'" + ANSI_RESET);
            }

            // only bind textures for uniforms declared in the shader
            const GLint loc = glGetUniformLocation(shader->getProgramID(), plane.getName().c_str());
            if (loc >= 0)
            {
                glActiveTexture(GL_TEXTURE0 + unit);
                texture->bind();
                glUniform1i(loc, unit);
                ++unit;
            }
            if (!texelSource) texelSource = texture;
        }
    }

    // optional inverse size for fullscreen passes (first bound texture)
    const GLint texelLoc = static_cast<GLint>(shader->getUniformLocation("uTexelSize"));
    if (texelLoc >= 0 && texelSource)
    {
        const float w = static_cast<float>(std::max(1, texelSource->getWidth()));
        const float h = static_cast<float>(std::max(1, texelSource->getHeight()));
        glUniform2f(texelLoc, 1.0f / w, 1.0f / h);
    }
}

void Target::beginPresent(Shader* shader, Filter presentFilter) const
{
    require(*this);
    if (!shader)
    {
        throw std::runtime_error(ANSI_RED + "[Target] present has a null shader" + ANSI_RESET);
    }

    // remember the color texture so endPresent can restore its filter
    Texture* texture = impl->soleColorTexture();
    impl->presentColor = texture;
    texture->setFilter(filterToGL(presentFilter), filterToGL(presentFilter));

    glActiveTexture(GL_TEXTURE0);
    texture->bind();
    const GLint loc = static_cast<GLint>(shader->getUniformLocation("uAlbedo"));
    if (loc >= 0) glUniform1i(loc, 0);
}

void Target::endPresent() const
{
    require(*this);
    if (!impl->presentColor) return;

    // restore offscreen sampling filter after present
    const GLenum stored = filterToGL(impl->filter);
    impl->presentColor->setFilter(stored, stored);
    impl->presentColor = nullptr;
}

} // namespace kerf
