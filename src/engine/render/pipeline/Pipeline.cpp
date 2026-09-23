#include <kerf/engine/render/pipeline/Pipeline.h>

#include <kerf/engine/render/buffer/FrameBuffer.h>
#include <kerf/engine/render/camera/Camera.h>
#include <kerf/engine/render/context/Context.h>
#include <kerf/engine/resource/ObjServer.h>
#include <kerf/engine/resource/ShaderServer.h>
#include <kerf/engine/resource/Texture.h>
#include <kerf/engine/scene/Scene.h>
#include <kerf/shared/Const.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <algorithm>
#include <stdexcept>
#include <type_traits>
#include <unordered_map>
#include <utility>


namespace kerf {

namespace {

const char* attachmentName(AttachmentKind kind)
{
    switch (kind)
    {
        case AttachmentKind::Color:
            return "color";
        case AttachmentKind::Normal:
            return "normal";
        case AttachmentKind::Depth:
            return "depth";
    }
    throw std::runtime_error(ANSI_RED + "[Pipeline] unknown attachment kind" + ANSI_RESET);
}

std::vector<AttachmentDesc> resolvedAttachments(const TargetDesc& desc)
{
    if (desc.attachments.empty())
    {
        return { AttachmentDesc{ AttachmentKind::Color, Format::RGBA8 } };
    }
    return desc.attachments;
}

void validateAttachment(const AttachmentDesc& attachment)
{
    const bool depthKind = attachment.kind == AttachmentKind::Depth;
    const bool depthFormat = attachment.format == Format::Depth24;
    if (depthKind != depthFormat)
    {
        throw std::runtime_error(
            ANSI_RED + "[Pipeline] Depth attachments require Depth24; color/normal cannot use Depth24" + ANSI_RESET
        );
    }
}

void formatsFor(Format format, GLenum& internalFormat, GLenum& formatEnum, GLenum& type)
{
    switch (format)
    {
        case Format::RGBA8:
            internalFormat = GL_RGBA8;
            formatEnum = GL_RGBA;
            type = GL_UNSIGNED_BYTE;
            return;
        case Format::RGB16F:
            // RGBA16F is color-renderable everywhere; RGB16F often is not (macOS).
            internalFormat = GL_RGBA16F;
            formatEnum = GL_RGBA;
            type = GL_FLOAT;
            return;
        case Format::Depth24:
            internalFormat = GL_DEPTH_COMPONENT24;
            formatEnum = GL_DEPTH_COMPONENT;
            type = GL_FLOAT;
            return;
    }
    throw std::runtime_error(ANSI_RED + "[Pipeline] unsupported format" + ANSI_RESET);
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
    throw std::runtime_error(ANSI_RED + "[Pipeline] unsupported filter" + ANSI_RESET);
}

void bindSampler(Shader* shader, const char* uniform, Texture* texture, int& unit)
{
    if (!shader || !texture) return;

    const GLint loc = glGetUniformLocation(shader->getProgramID(), uniform);
    if (loc < 0) return;

    glActiveTexture(GL_TEXTURE0 + unit);
    texture->bind();
    glUniform1i(loc, unit);
    ++unit;
}

void bindConventionSamplers(Shader* shader, const std::unordered_map<std::string, Texture*>& attachments)
{
    auto find = [&](const char* name) -> Texture* {
        const auto it = attachments.find(name);
        if (it == attachments.end()) return nullptr;
        return it->second;
    };

    // Bind only samplers the program actually uses, starting at unit 0.
    // Otherwise a post shader with just uNormal/uDepth would sample leftover
    // mesh albedo still bound to GL_TEXTURE0 (sampler default).
    int unit = 0;
    bindSampler(shader, "uColor", find("color"), unit);
    bindSampler(shader, "uNormal", find("normal"), unit);
    bindSampler(shader, "uDepth", find("depth"), unit);

    Texture* texelSource = find("color");
    if (!texelSource) texelSource = find("normal");
    if (!texelSource) texelSource = find("depth");

    const GLint texelLoc = static_cast<GLint>(shader->getUniformLocation("uTexelSize"));
    if (texelLoc >= 0 && texelSource)
    {
        const float w = static_cast<float>(std::max(1, texelSource->getWidth()));
        const float h = static_cast<float>(std::max(1, texelSource->getHeight()));
        glUniform2f(texelLoc, 1.0f / w, 1.0f / h);
    }
}

void drawFullscreen()
{
    const GLboolean depth = glIsEnabled(GL_DEPTH_TEST);
    const GLboolean blend = glIsEnabled(GL_BLEND);
    const GLboolean msaa = glIsEnabled(GL_MULTISAMPLE);
    if (depth) glDisable(GL_DEPTH_TEST);
    if (blend) glDisable(GL_BLEND);
    if (msaa) glDisable(GL_MULTISAMPLE);

    ObjServer::getMesh("unit")->draw();

    if (depth) glEnable(GL_DEPTH_TEST);
    if (blend) glEnable(GL_BLEND);
    if (msaa) glEnable(GL_MULTISAMPLE);
}

} // namespace

ScenePass::ScenePass(std::string target) : target(std::move(target)) {}

FullscreenPass::FullscreenPass(Shader* shader, std::string input, std::string output)
    : shader(shader), input(std::move(input)), output(std::move(output))
{}

PresentPass::PresentPass(std::string source) : source(std::move(source)) {}

Pipeline::Pipeline() = default;
Pipeline::~Pipeline() = default;

void Pipeline::addTarget(const std::string& name, const TargetDesc& desc)
{
    if (targets.find(name) != targets.end())
    {
        throw std::runtime_error(ANSI_RED + "[Pipeline] duplicate target '" + name + "'" + ANSI_RESET);
    }
    if (desc.scale <= 0.0f)
    {
        throw std::runtime_error(ANSI_RED + "[Pipeline] target '" + name + "' scale must be positive" + ANSI_RESET);
    }

    const std::vector<AttachmentDesc> resolved = resolvedAttachments(desc);
    bool seenColor = false;
    bool seenNormal = false;
    bool seenDepth = false;
    for (const AttachmentDesc& attachment : resolved)
    {
        validateAttachment(attachment);
        switch (attachment.kind)
        {
            case AttachmentKind::Color:
                if (seenColor)
                {
                    throw std::runtime_error(ANSI_RED + "[Pipeline] target '" + name + "' has duplicate color" + ANSI_RESET);
                }
                seenColor = true;
                break;
            case AttachmentKind::Normal:
                if (seenNormal)
                {
                    throw std::runtime_error(ANSI_RED + "[Pipeline] target '" + name + "' has duplicate normal" + ANSI_RESET);
                }
                seenNormal = true;
                break;
            case AttachmentKind::Depth:
                if (seenDepth)
                {
                    throw std::runtime_error(ANSI_RED + "[Pipeline] target '" + name + "' has duplicate depth" + ANSI_RESET);
                }
                seenDepth = true;
                break;
        }
    }

    RenderTarget rt;
    rt.desc = desc;
    rt.desc.attachments = resolved;
    targets.emplace(name, std::move(rt));
    lastFramebufferWidth = 0;
    lastFramebufferHeight = 0;
}

void Pipeline::add(ScenePass pass)
{
    passes.emplace_back(std::move(pass));
}

void Pipeline::add(FullscreenPass pass)
{
    passes.emplace_back(std::move(pass));
}

void Pipeline::add(PresentPass pass)
{
    passes.emplace_back(std::move(pass));
}

void Pipeline::ensureResources(int framebufferWidth, int framebufferHeight)
{
    if (framebufferWidth == lastFramebufferWidth && framebufferHeight == lastFramebufferHeight)
    {
        return;
    }

    lastFramebufferWidth = framebufferWidth;
    lastFramebufferHeight = framebufferHeight;

    for (auto& [name, rt] : targets)
    {
        const int width = std::max(1, static_cast<int>(framebufferWidth * rt.desc.scale));
        const int height = std::max(1, static_cast<int>(framebufferHeight * rt.desc.scale));
        const std::vector<AttachmentDesc>& specs = rt.desc.attachments;

        if (!rt.fbo)
        {
            rt.fbo = std::make_unique<FrameBuffer>();
        }

        if (rt.textures.size() != specs.size())
        {
            rt.textures.clear();
            rt.attachments.clear();
            rt.textures.reserve(specs.size());
            for (const AttachmentDesc& spec : specs)
            {
                GLenum internalFormat = GL_RGBA8;
                GLenum formatEnum = GL_RGBA;
                GLenum type = GL_UNSIGNED_BYTE;
                formatsFor(spec.format, internalFormat, formatEnum, type);

                auto texture = std::make_unique<Texture>(width, height, internalFormat, formatEnum, type);
                const GLenum filter = spec.kind == AttachmentKind::Depth
                    ? GL_NEAREST
                    : filterToGL(rt.desc.filter);
                texture->setFilter(filter, filter);
                rt.attachments[attachmentName(spec.kind)] = texture.get();
                rt.textures.push_back(std::move(texture));
            }
        }
        else
        {
            for (size_t i = 0; i < specs.size(); ++i)
            {
                rt.textures[i]->resize(width, height);
                const GLenum filter = specs[i].kind == AttachmentKind::Depth
                    ? GL_NEAREST
                    : filterToGL(rt.desc.filter);
                rt.textures[i]->setFilter(filter, filter);
                rt.attachments[attachmentName(specs[i].kind)] = rt.textures[i].get();
            }
        }

        std::vector<FrameBuffer::Attachment> fboAttachments;
        fboAttachments.reserve(specs.size());
        for (const AttachmentDesc& spec : specs)
        {
            fboAttachments.push_back(FrameBuffer::Attachment{
                attachmentName(spec.kind),
                rt.attachments[attachmentName(spec.kind)]
            });
        }
        rt.fbo->setAttachments(fboAttachments);
    }
}

void Pipeline::validate() const
{
    for (const auto& pass : passes)
    {
        std::visit([&](const auto& p) {
            using T = std::decay_t<decltype(p)>;
            if constexpr (std::is_same_v<T, ScenePass>)
            {
                requireTarget(p.target);
            }
            else if constexpr (std::is_same_v<T, FullscreenPass>)
            {
                if (!p.shader)
                {
                    throw std::runtime_error(ANSI_RED + "[Pipeline] FullscreenPass has a null shader" + ANSI_RESET);
                }
                requireTarget(p.input);
                requireTarget(p.output);
            }
            else if constexpr (std::is_same_v<T, PresentPass>)
            {
                const RenderTarget& rt = requireTarget(p.source);
                if (rt.attachments.find("color") == rt.attachments.end())
                {
                    throw std::runtime_error(
                        ANSI_RED + "[Pipeline] PresentPass source '" + p.source + "' has no color attachment" + ANSI_RESET
                    );
                }
            }
        }, pass);
    }
}

Pipeline::RenderTarget& Pipeline::requireTarget(const std::string& name)
{
    const auto it = targets.find(name);
    if (it == targets.end())
    {
        throw std::runtime_error(ANSI_RED + "[Pipeline] unknown target '" + name + "'" + ANSI_RESET);
    }
    return it->second;
}

const Pipeline::RenderTarget& Pipeline::requireTarget(const std::string& name) const
{
    const auto it = targets.find(name);
    if (it == targets.end())
    {
        throw std::runtime_error(ANSI_RED + "[Pipeline] unknown target '" + name + "'" + ANSI_RESET);
    }
    return it->second;
}

void Pipeline::runScene(const ScenePass& pass, Scene* scene)
{
    if (!scene) return;

    RenderTarget& rt = requireTarget(pass.target);
    rt.fbo->bind();

    Camera* camera = scene->getCamera();
    if (camera) camera->resize(rt.fbo->getWidth(), rt.fbo->getHeight());

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    if (rt.attachments.find("normal") != rt.attachments.end())
    {
        const GLfloat normalClear[] = { 0.0f, 0.0f, 1.0f, 1.0f };
        glClearBufferfv(GL_COLOR, 1, normalClear);
    }

    const GLboolean blend = glIsEnabled(GL_BLEND);
    if (blend) glDisable(GL_BLEND);

    scene->draw();

    if (blend) glEnable(GL_BLEND);
    rt.fbo->unbind();
}

void Pipeline::runFullscreen(const FullscreenPass& pass, Scene* scene)
{
    const RenderTarget& input = requireTarget(pass.input);
    RenderTarget& output = requireTarget(pass.output);

    output.fbo->bind();
    glClear(GL_COLOR_BUFFER_BIT);

    pass.shader->bind();
    bindConventionSamplers(pass.shader, input.attachments);

    Camera* camera = scene ? scene->getCamera() : nullptr;
    if (camera)
    {
        const GLuint program = pass.shader->getProgramID();
        const GLint invLoc = glGetUniformLocation(program, "uInvProjection");
        if (invLoc >= 0)
        {
            const glm::mat4 invProjection = glm::inverse(camera->getProjection());
            glUniformMatrix4fv(invLoc, 1, GL_FALSE, glm::value_ptr(invProjection));
        }
        const GLint nearLoc = glGetUniformLocation(program, "uNear");
        if (nearLoc >= 0) glUniform1f(nearLoc, camera->getNear());
        const GLint farLoc = glGetUniformLocation(program, "uFar");
        if (farLoc >= 0) glUniform1f(farLoc, camera->getFar());
    }

    drawFullscreen();
    output.fbo->unbind();
}

void Pipeline::runPresent(const PresentPass& pass, Context& context, int windowWidth, int windowHeight)
{
    const RenderTarget& source = requireTarget(pass.source);
    Texture* color = nullptr;
    const auto it = source.attachments.find("color");
    if (it != source.attachments.end()) color = it->second;

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, windowWidth, windowHeight);

    if (!color)
    {
        context.swapBuffers();
        return;
    }

    Shader* shader = ShaderServer::getShader("present");
    shader->bind();

    glActiveTexture(GL_TEXTURE0);
    color->bind();
    const GLint loc = static_cast<GLint>(shader->getUniformLocation("uAlbedo"));
    if (loc >= 0) glUniform1i(loc, 0);

    drawFullscreen();
    context.swapBuffers();
}

void Pipeline::execute(Scene* scene, Context& context)
{
    int framebufferWidth = 0;
    int framebufferHeight = 0;
    glfwGetFramebufferSize(context.getWindow(), &framebufferWidth, &framebufferHeight);

    ensureResources(framebufferWidth, framebufferHeight);
    validate();

    for (const auto& pass : passes)
    {
        std::visit([&](const auto& p) {
            using T = std::decay_t<decltype(p)>;
            if constexpr (std::is_same_v<T, ScenePass>)
            {
                runScene(p, scene);
            }
            else if constexpr (std::is_same_v<T, FullscreenPass>)
            {
                runFullscreen(p, scene);
            }
            else if constexpr (std::is_same_v<T, PresentPass>)
            {
                runPresent(p, context, framebufferWidth, framebufferHeight);
            }
        }, pass);
    }
}

} // namespace kerf
