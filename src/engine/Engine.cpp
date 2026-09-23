#include <kerf/engine/Engine.h>

#include <kerf/engine/render/pipeline/Pipeline.h>
#include <kerf/engine/scene/Scene.h>
#include <kerf/engine/resource/ObjServer.h>
#include <kerf/engine/resource/ShaderServer.h>
#include <kerf/engine/resource/TextureServer.h>
#include "EmbeddedShaders.h"
#include "EmbeddedMeshes.h"
#include "EmbeddedImages.h"


namespace kerf {

Engine::Engine(int width, int height) : 
    context(width, height), 
    mouse(context.getWindow()),
    keyboard(context.getWindow())
{
    context.setTitle("Kerfengine");
    context.setClearColor(glm::vec4(0.12f, 0.12f, 0.14f, 1.0f));
    ObjServer::loadMeshFromSource("unit", embedded::unit_obj);
    ShaderServer::loadShaderFromSource("default2d", embedded::default2d_vert, embedded::default2d_frag);
    ShaderServer::loadShaderFromSource("default3d", embedded::default3d_vert, embedded::default3d_frag);
    ShaderServer::loadShaderFromSource("text", embedded::text_vert, embedded::text_frag);
    ShaderServer::loadShaderFromSource("present", embedded::present_vert, embedded::present_frag);
    TextureServer::loadTextureFromMemory("white", embedded::white_png, static_cast<int>(embedded::white_png_len));

    defaultPipeline = std::make_unique<Pipeline>();
    defaultPipeline->addTarget("scene", TargetDesc{});
    defaultPipeline->add(ScenePass("scene"));
    defaultPipeline->add(PresentPass("scene"));
    pipeline = defaultPipeline.get();
}

Engine::~Engine()
{
    // No Op
}

void Engine::setScene(Scene* scene)
{
    this->scene = scene;
}

void Engine::setPipeline(Pipeline* pipeline)
{
    this->pipeline = pipeline ? pipeline : defaultPipeline.get();
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

void Engine::render()
{
    pipeline->execute(scene, context);
}

void Engine::update()
{
    glfwPollEvents();
    mouse.update();
    keyboard.update();
    if (scene) scene->update(0.016f, mouse); // 60 fps TODO real time
}

bool Engine::shouldClose() const
{
    return context.shouldClose();
}

} // namespace kerf
