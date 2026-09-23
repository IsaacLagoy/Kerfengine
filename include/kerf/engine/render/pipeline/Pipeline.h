#pragma once

#include <kerf/engine/render/pipeline/Format.h>

#include <memory>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>


namespace kerf {

class Shader;
class Scene;
class Context;
class Texture;
class FrameBuffer;

struct AttachmentDesc {
    AttachmentKind kind = AttachmentKind::Color;
    Format format = Format::RGBA8;
};

struct TargetDesc {
    float scale = 1.0f;
    Filter filter = Filter::Linear;
    std::vector<AttachmentDesc> attachments;
};

struct ScenePass {
    std::string target;
    explicit ScenePass(std::string target);
};

struct FullscreenPass {
    Shader* shader = nullptr;
    std::string input;
    std::string output;
    FullscreenPass(Shader* shader, std::string input, std::string output);
};

struct PresentPass {
    std::string source;
    explicit PresentPass(std::string source);
};

class Pipeline {
public:
    Pipeline();
    ~Pipeline();

    Pipeline(const Pipeline&) = delete;
    Pipeline& operator=(const Pipeline&) = delete;
    Pipeline(Pipeline&&) = delete;
    Pipeline& operator=(Pipeline&&) = delete;

    void addTarget(const std::string& name, const TargetDesc& desc);
    void add(ScenePass pass);
    void add(FullscreenPass pass);
    void add(PresentPass pass);

    void execute(Scene* scene, Context& context);

private:
    struct RenderTarget {
        TargetDesc desc;
        std::vector<std::unique_ptr<Texture>> textures;
        std::unique_ptr<FrameBuffer> fbo;
        std::unordered_map<std::string, Texture*> attachments;
    };

    using Pass = std::variant<ScenePass, FullscreenPass, PresentPass>;

    std::unordered_map<std::string, RenderTarget> targets;
    std::vector<Pass> passes;
    int lastFramebufferWidth = 0;
    int lastFramebufferHeight = 0;

    void ensureResources(int framebufferWidth, int framebufferHeight);
    void validate() const;
    RenderTarget& requireTarget(const std::string& name);
    const RenderTarget& requireTarget(const std::string& name) const;

    void runScene(const ScenePass& pass, Scene* scene);
    void runFullscreen(const FullscreenPass& pass, Scene* scene);
    void runPresent(const PresentPass& pass, Context& context, int windowWidth, int windowHeight);
};

} // namespace kerf
