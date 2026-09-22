#include <kerf/engine/Engine.h>

#include <kerf/engine/render/buffer/FrameBuffer.h>
#include <kerf/engine/scene/Scene.h>
#include <kerf/engine/render/camera/Camera.h>
#include <kerf/engine/resource/ObjServer.h>
#include <kerf/engine/resource/ShaderServer.h>
#include <kerf/engine/resource/Texture.h>
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

} // namespace kerf