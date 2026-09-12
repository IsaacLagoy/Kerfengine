#include "engine/Engine.h"
#include "engine/render/buffer/FrameBuffer.h"
#include "engine/scene/Scene.h"
#include "engine/render/camera/Camera.h"
#include "engine/resource/ObjServer.h"
#include "engine/resource/ShaderServer.h"
#include "engine/resource/Texture.h"
#include "engine/resource/TextureServer.h"


Engine::Engine(int width, int height) : 
    context(width, height), 
    mouse(context.getWindow()),
    keyboard(context.getWindow())
{
    context.setTitle("Kerfengine");
    glClearColor(0.12f, 0.12f, 0.14f, 1.0f);
    ObjServer::loadMesh("unit", "resources/mesh/unit.obj");
    ShaderServer::loadShader("default2d", "shaders/default2d.vert", "shaders/default2d.frag");
    ShaderServer::loadShader("default3d", "shaders/default3d.vert", "shaders/default3d.frag");
    ShaderServer::loadShader("text", "shaders/text.vert", "shaders/text.frag");
    ShaderServer::loadShader("present", "shaders/present.vert", "shaders/present.frag");
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
    // can only render if there is a scene
    if (!scene) return;

    int framebufferWidth = 0;
    int framebufferHeight = 0;
    glfwGetFramebufferSize(context.getWindow(), &framebufferWidth, &framebufferHeight);

    Camera* camera = scene->getCamera();
    if (camera) camera->resize(framebufferWidth, framebufferHeight);

    if (fbo)
    {
        fbo->bind();
    }
    else
    {
        glViewport(0, 0, framebufferWidth, framebufferHeight);
    }

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    scene->draw();

    if (fbo)
    {
        fbo->unbind();
        return;
    }

    context.swapBuffers();
}

// TODO dekludge this function
void Engine::present()
{
    int framebufferWidth = 0;
    int framebufferHeight = 0;
    glfwGetFramebufferSize(context.getWindow(), &framebufferWidth, &framebufferHeight);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, framebufferWidth, framebufferHeight);

    if (!fbo || !fbo->getTexture())
    {
        context.swapBuffers();
        return;
    }

    // Window is MSAA; a blit from this non-MSAA FBO is invalid.
    const GLboolean depth = glIsEnabled(GL_DEPTH_TEST);
    const GLboolean blend = glIsEnabled(GL_BLEND);
    const GLboolean msaa = glIsEnabled(GL_MULTISAMPLE);
    if (depth) glDisable(GL_DEPTH_TEST);
    if (blend) glDisable(GL_BLEND);
    if (msaa) glDisable(GL_MULTISAMPLE);

    Shader* shader = ShaderServer::getShader("present");
    shader->bind();

    glActiveTexture(GL_TEXTURE0);
    fbo->getTexture()->bind();
    const GLint loc = static_cast<GLint>(shader->getUniformLocation("uAlbedo"));
    if (loc >= 0) glUniform1i(loc, 0);

    ObjServer::getMesh("unit")->draw();

    if (depth) glEnable(GL_DEPTH_TEST);
    if (blend) glEnable(GL_BLEND);
    if (msaa) glEnable(GL_MULTISAMPLE);

    context.swapBuffers();
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