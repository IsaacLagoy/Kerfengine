#include <kerf/engine/Engine.h>

#include "engine/render/pipeline/RenderPasses.h"

#include <kerf/engine/scene/Scene.h>
#include <kerf/engine/resource/ObjServer.h>
#include <kerf/engine/resource/ShaderServer.h>
#include <kerf/engine/resource/TextureServer.h>
#include "EmbeddedShaders.h"
#include "EmbeddedMeshes.h"
#include "EmbeddedImages.h"

#include <span>


namespace kerf {

Engine::Engine(int width, int height) : 
    context(width, height), 
    mouse(context.getWindow()),
    keyboard(context.getWindow()),
    renderer(std::make_unique<RenderSystem>())
{
    context.setTitle("Kerfengine");
    context.setClearColor(glm::vec4(0.12f, 0.12f, 0.14f, 1.0f));
    ObjServer::loadMeshFromSource("unit", embedded::unit_obj);
    ShaderServer::loadShaderFromSource("default2d", embedded::default2d_vert, embedded::default2d_frag);
    ShaderServer::loadShaderFromSource("default3d", embedded::default3d_vert, embedded::default3d_frag);
    ShaderServer::loadShaderFromSource("text", embedded::text_vert, embedded::text_frag);
    ShaderServer::loadShaderFromSource("present", embedded::present_vert, embedded::present_frag);
    TextureServer::loadTextureFromMemory("white", embedded::white_png, static_cast<int>(embedded::white_png_len));
}

Engine::~Engine() = default;

void Engine::framebufferSize(int& width, int& height) const
{
    glfwGetFramebufferSize(context.getWindow(), &width, &height);
}

void Engine::setScene(Scene* scene)
{
    this->scene = scene;
}

void Engine::setClearColor(const glm::vec4& color)
{
    context.setClearColor(color);
}

void Engine::setTitle(const std::string& title)
{
    context.setTitle(title);
}

Mouse& Engine::getMouse()
{
    return mouse;
}

Keyboard& Engine::getKeyboard()
{
    return keyboard;
}

float Engine::getDeltaTime() const
{
    return deltaTime;
}

void Engine::scenePass(Target output)
{
    int width = 0;
    int height = 0;
    framebufferSize(width, height);
    renderer->scenePass(scene, width, height, output);
}

void Engine::postPass(
    Shader* shader,
    std::initializer_list<Target> inputs,
    Target output
)
{
    int width = 0;
    int height = 0;
    framebufferSize(width, height);
    renderer->postPass(
        scene,
        width,
        height,
        shader,
        std::span<const Target>(inputs.begin(), inputs.end()),
        output
    );
}

void Engine::present(Target color, PresentFit fit, Filter filter)
{
    int width = 0;
    int height = 0;
    framebufferSize(width, height);
    renderer->present(context, width, height, color, fit, filter);
}

void Engine::update()
{
    const double now = glfwGetTime();
    deltaTime = hasPreviousTime ? static_cast<float>(now - previousTime) : 0.0f;
    previousTime = now;
    hasPreviousTime = true;

    glfwPollEvents();
    mouse.update();
    keyboard.update();
    if (scene) scene->update(deltaTime, mouse);
}

bool Engine::shouldClose() const
{
    return context.shouldClose();
}

} // namespace kerf
