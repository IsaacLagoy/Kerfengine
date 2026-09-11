#include "engine/Engine.h"
#include "engine/render/buffer/FrameBuffer.h"
#include "engine/scene/Scene.h"
#include "engine/render/camera/Camera.h"
#include "engine/resource/ObjServer.h"
#include "engine/resource/ShaderServer.h"
#include "engine/resource/TextureServer.h"


Engine::Engine(int width, int height) : context(width, height)
{
    context.setTitle("Kerfengine");
    glClearColor(0.12f, 0.12f, 0.14f, 1.0f);
    ObjServer::loadMesh("unit", "resources/mesh/unit.obj");
    ShaderServer::loadShader("default2d", "shaders/default2d.vert", "shaders/default2d.frag");
    ShaderServer::loadShader("text", "shaders/text.vert", "shaders/text.frag");
    TextureServer::loadTexture("white", "resources/image/white.png");
}

Engine::~Engine()
{
    // No Op
}

void Engine::setScene(Scene* scene)
{
    this->scene = scene;
}

void Engine::setFBO(FrameBuffer* fbo)
{
    this->fbo = fbo;
}

void Engine::render()
{
    // can only render if there is a scene
    if (!scene) return;

    // bind the fbo if it exists
    if (fbo) fbo->bind();

    int framebufferWidth = 0;
    int framebufferHeight = 0;
    glfwGetFramebufferSize(context.getWindow(), &framebufferWidth, &framebufferHeight);
    glViewport(0, 0, framebufferWidth, framebufferHeight);

    Camera* camera = scene->getCamera();
    if (camera) camera->resize(framebufferWidth, framebufferHeight);

    // clear the screen
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // draw the scene
    scene->draw();

    // unbind the fbo if it exists
    if (fbo) fbo->unbind();

    // swap the buffers
    context.swapBuffers();
}

void Engine::update()
{
    if (scene) scene->update(0.016f); // 60 fps

    glfwPollEvents();
}

bool Engine::shouldClose() const
{
    return context.shouldClose();
}