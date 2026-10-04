#pragma once

#include <kerf/engine/render/pipeline/Target.h>
#include <span>


namespace kerf {

class Scene;
class Shader;
class Context;

class RenderSystem {
public:
    void scenePass(
        Scene* scene,
        int framebufferWidth,
        int framebufferHeight,
        Target output
    );

    void postPass(
        Scene* scene,
        int framebufferWidth,
        int framebufferHeight,
        Shader* shader,
        std::span<const Target> inputs,
        Target output
    );

    void present(
        Context& context,
        int windowWidth,
        int windowHeight,
        Target color,
        PresentFit fit,
        Filter filter
    );
};

} // namespace kerf
