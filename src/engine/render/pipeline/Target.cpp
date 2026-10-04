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
            internalFormat = GL_RGBA8;
            formatEnum = GL_RGBA;
            type = GL_UNSIGNED_BYTE;
            return;
        case Format::RGB16F:
            internalFormat = GL_RGBA16F;
            formatEnum = GL_RGBA;
            type = GL_FLOAT;
            return;
        case Format::Depth24:
            internalFormat = GL_DEPTH_COMPONENT24;
            formatEnum = GL_DEPTH_COMPONENT;
            type = GL_FLOAT;
            return;
        case Format::UnsignedInt8:
            internalFormat = GL_R8UI;
            formatEnum = GL_RED_INTEGER;
            type = GL_UNSIGNED_BYTE;
            return;
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

    if (size.mode == TargetSize::Mode::Scale)
    {
        return glm::ivec2(
            std::max(1, static_cast<int>(fbW * size.scale)),
            std::max(1, static_cast<int>(fbH * size.scale))
        );
    }

    const int width = size.width;
    int height = size.height;
    if (height <= 0)
    {
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

struct Target::Impl {
    struct StoredPlane {
        std::string name;
        Format format = Format::RGBA8;
        glm::vec4 clear{0.0f, 0.0f, 0.0f, 0.0f};
        bool clearBeforeRender = true;
        std::unique_ptr<Texture> texture;
        int colorIndex = -1;
    };

    TargetSize size;
    Filter filter = Filter::Linear;
    std::vector<StoredPlane> planes;
    std::unique_ptr<FrameBuffer> fbo;
    int width = 0;
    int height = 0;
    Texture* presentColor = nullptr;

    StoredPlane* findPlane(const std::string& name)
    {
        for (StoredPlane& plane : planes)
        {
            if (plane.name == name) return &plane;
        }
        return nullptr;
    }

    Texture* soleColorTexture() const
    {
        Texture* color = nullptr;
        for (const StoredPlane& plane : planes)
        {
            if (plane.format == Format::Depth24) continue;
            if (color)
            {
                throw std::runtime_error(
                    ANSI_RED + "[Target] present requires exactly one color plane" + ANSI_RESET
                );
            }
            color = plane.texture.get();
        }
        if (!color)
        {
            throw std::runtime_error(
                ANSI_RED + "[Target] present requires exactly one color plane" + ANSI_RESET
            );
        }
        return color;
    }

    void clearBound(const StoredPlane& plane) const
    {
        if (plane.format == Format::Depth24)
        {
            const GLfloat depth = plane.clear.r;
            glClearBufferfv(GL_DEPTH, 0, &depth);
            return;
        }

        if (plane.format == Format::UnsignedInt8)
        {
            const GLuint value[4] = {
                static_cast<GLuint>(plane.clear.r + 0.5f),
                static_cast<GLuint>(plane.clear.g + 0.5f),
                static_cast<GLuint>(plane.clear.b + 0.5f),
                static_cast<GLuint>(plane.clear.a + 0.5f)
            };
            glClearBufferuiv(GL_COLOR, plane.colorIndex, value);
            return;
        }

        const GLfloat value[4] = { plane.clear.r, plane.clear.g, plane.clear.b, plane.clear.a };
        glClearBufferfv(GL_COLOR, plane.colorIndex, value);
    }

    void allocate(int w, int h)
    {
        width = w;
        height = h;

        int colorIndex = 0;
        std::vector<FrameBuffer::Attachment> attachments;
        attachments.reserve(planes.size());

        for (StoredPlane& plane : planes)
        {
            GLenum internalFormat = GL_RGBA8;
            GLenum formatEnum = GL_RGBA;
            GLenum type = GL_UNSIGNED_BYTE;
            glFormatsFor(plane.format, internalFormat, formatEnum, type);

            const GLenum glFilter = plane.format == Format::Depth24 ? GL_NEAREST : filterToGL(filter);
            if (!plane.texture)
            {
                plane.texture = std::make_unique<Texture>(width, height, internalFormat, formatEnum, type);
            }
            else
            {
                plane.texture->resize(width, height);
            }
            plane.texture->setFilter(glFilter, glFilter);

            if (plane.format == Format::Depth24)
            {
                plane.colorIndex = -1;
            }
            else
            {
                plane.colorIndex = colorIndex++;
            }

            attachments.push_back(FrameBuffer::Attachment{
                plane.name,
                plane.texture.get(),
                plane.format
            });
        }

        if (!fbo)
        {
            fbo = std::make_unique<FrameBuffer>();
        }
        fbo->setAttachments(attachments);
    }
};

Target::Target(TargetSize size, Filter filter, std::initializer_list<Plane> planes)
    : impl(std::make_shared<Impl>())
{
    if (planes.size() == 0)
    {
        throw std::runtime_error(ANSI_RED + "[Target] at least one plane is required" + ANSI_RESET);
    }
    validateSize(size);

    std::unordered_set<std::string> names;
    bool haveDepth = false;
    impl->planes.reserve(planes.size());
    for (Plane plane : planes)
    {
        if (plane.name.empty())
        {
            throw std::runtime_error(ANSI_RED + "[Target] name must not be empty" + ANSI_RESET);
        }
        if (!names.insert(plane.name).second)
        {
            throw std::runtime_error(
                ANSI_RED + "[Target] duplicate plane name '" + plane.name + "'" + ANSI_RESET
            );
        }
        if (plane.format == Format::Depth24)
        {
            if (haveDepth)
            {
                throw std::runtime_error(ANSI_RED + "[Target] at most one depth plane is allowed" + ANSI_RESET);
            }
            haveDepth = true;
            if (plane.clear == glm::vec4(0.0f))
            {
                plane.clear = glm::vec4(1.0f, 0.0f, 0.0f, 0.0f);
            }
        }

        Impl::StoredPlane stored;
        stored.name = std::move(plane.name);
        stored.format = plane.format;
        stored.clear = plane.clear;
        stored.clearBeforeRender = plane.clearBeforeRender;
        impl->planes.push_back(std::move(stored));
    }

    impl->size = size;
    impl->filter = filter;
    impl->allocate(1, 1);
}

Target::Target(TargetSize size, Format format, Filter filter, std::string name)
    : Target(size, filter, { Plane{ std::move(name), format } })
{
}

Target::~Target() = default;

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
    impl->fbo->bind();
    for (const Impl::StoredPlane& plane : impl->planes)
    {
        if (plane.clearBeforeRender)
        {
            impl->clearBound(plane);
        }
    }
}

void Target::endOutput() const
{
    require(*this);
    impl->fbo->unbind();
}

void Target::clearPlane(const std::string& name) const
{
    require(*this);
    Impl::StoredPlane* plane = impl->findPlane(name);
    if (!plane)
    {
        throw std::runtime_error(
            ANSI_RED + "[Target] no plane named '" + name + "'" + ANSI_RESET
        );
    }

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

    for (const Target& input : inputs)
    {
        require(input);
        for (const Impl::StoredPlane& plane : input.impl->planes)
        {
            Texture* texture = plane.texture.get();
            if (!texture)
            {
                throw std::runtime_error(
                    ANSI_RED + "[Target] null plane '" + plane.name + "'" + ANSI_RESET
                );
            }
            if (!textures.insert(texture).second)
            {
                throw std::runtime_error(
                    ANSI_RED + "[Target] duplicate input plane '" + plane.name + "'" + ANSI_RESET
                );
            }
            if (!names.insert(plane.name).second)
            {
                throw std::runtime_error(
                    ANSI_RED + "[Target] duplicate input name '" + plane.name + "'" + ANSI_RESET
                );
            }

            const GLint loc = glGetUniformLocation(shader->getProgramID(), plane.name.c_str());
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

    const GLenum stored = filterToGL(impl->filter);
    impl->presentColor->setFilter(stored, stored);
    impl->presentColor = nullptr;
}

} // namespace kerf
