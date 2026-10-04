#include "RenderPasses.h"

#include <kerf/engine/render/camera/Camera.h>
#include <kerf/engine/render/context/Context.h>
#include <kerf/engine/resource/ObjServer.h>
#include <kerf/engine/resource/ShaderServer.h>
#include <kerf/engine/scene/Scene.h>
#include <kerf/shared/Const.h>

#include <glad/glad.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <algorithm>
#include <stdexcept>


namespace kerf {

namespace {

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

void bindCameraUniforms(Shader* shader, Scene* scene)
{
    Camera* camera = scene ? scene->getCamera() : nullptr;
    if (!camera) return;

    const GLuint program = shader->getProgramID();
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

} // namespace

void RenderSystem::scenePass(
    Scene* scene,
    int framebufferWidth,
    int framebufferHeight,
    Target output
)
{
    if (!scene) return;
    if (!output)
    {
        throw std::runtime_error(ANSI_RED + "[Engine] scenePass requires an output target" + ANSI_RESET);
    }

    output.ensureSize(framebufferWidth, framebufferHeight);
    Camera* camera = scene->getCamera();
    if (camera) camera->resize(output.width(), output.height());

    output.beginOutput();

    const GLboolean blend = glIsEnabled(GL_BLEND);
    if (blend) glDisable(GL_BLEND);

    scene->draw();

    if (blend) glEnable(GL_BLEND);
    output.endOutput();
}

void RenderSystem::postPass(
    Scene* scene,
    int framebufferWidth,
    int framebufferHeight,
    Shader* shader,
    std::span<const Target> inputs,
    Target output
)
{
    if (!shader)
    {
        throw std::runtime_error(ANSI_RED + "[Engine] postPass has a null shader" + ANSI_RESET);
    }
    if (!output)
    {
        throw std::runtime_error(ANSI_RED + "[Engine] postPass requires an output target" + ANSI_RESET);
    }

    for (const Target& input : inputs)
    {
        input.ensureSize(framebufferWidth, framebufferHeight);
    }
    output.ensureSize(framebufferWidth, framebufferHeight);

    output.beginOutput();
    shader->bind();
    Target::bindSamplers(shader, inputs);
    bindCameraUniforms(shader, scene);
    drawFullscreen();
    output.endOutput();
}

void RenderSystem::present(
    Context& context,
    int windowWidth,
    int windowHeight,
    Target color,
    PresentFit fit,
    Filter filter
)
{
    if (!color)
    {
        throw std::runtime_error(ANSI_RED + "[Engine] present requires a color target" + ANSI_RESET);
    }

    color.ensureSize(windowWidth, windowHeight);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, windowWidth, windowHeight);

    GLfloat previousClear[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
    glGetFloatv(GL_COLOR_CLEAR_VALUE, previousClear);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glClearColor(previousClear[0], previousClear[1], previousClear[2], previousClear[3]);

    int vpX = 0;
    int vpY = 0;
    int vpW = windowWidth;
    int vpH = windowHeight;
    if (fit == PresentFit::Letterbox)
    {
        const float srcW = static_cast<float>(std::max(1, color.width()));
        const float srcH = static_cast<float>(std::max(1, color.height()));
        const float srcAspect = srcW / srcH;
        const float dstAspect = static_cast<float>(std::max(1, windowWidth))
            / static_cast<float>(std::max(1, windowHeight));
        if (srcAspect > dstAspect)
        {
            vpW = windowWidth;
            vpH = std::max(1, static_cast<int>(windowWidth / srcAspect + 0.5f));
            vpY = (windowHeight - vpH) / 2;
        }
        else
        {
            vpH = windowHeight;
            vpW = std::max(1, static_cast<int>(windowHeight * srcAspect + 0.5f));
            vpX = (windowWidth - vpW) / 2;
        }
    }
    glViewport(vpX, vpY, vpW, vpH);

    Shader* shader = ShaderServer::getShader("present");
    shader->bind();
    color.beginPresent(shader, filter);
    drawFullscreen();
    color.endPresent();
    context.swapBuffers();
}

} // namespace kerf
