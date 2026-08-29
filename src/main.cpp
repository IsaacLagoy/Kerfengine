#include "engine/scene/Scene.h"
#include "engine/resource/ObjServer.h"
#include "engine/resource/ShaderServer.h"
#include "engine/node/Model.h"
#include "engine/render/context/Context.h"
#include "engine/render/camera/Camera.h"

int main()
{
    const int width = 800;
    const int height = 600;

    Context context(width, height);
    context.setTitle("2D scene test");
    glClearColor(0.12f, 0.12f, 0.14f, 1.0f);

    ObjServer::loadMesh("quad", "resources/quad.obj");
    ShaderServer::loadShader("default2d", "shaders/default2d.vert", "shaders/default2d.frag");

    // create scene
    Camera camera(width, height, 1.0f);
    Scene scene;
    scene.setCamera(&camera);

    Mesh* quad = ObjServer::getMesh("quad");

    // Back plate: large, lowest layer
    Model* backdrop = new Model(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec2(0.7f, 0.45f), quad);
    backdrop->setColor(glm::vec4(0.22f, 0.24f, 0.32f, 1.0f));
    backdrop->setLayer(-1.0f);
    scene.addNode(backdrop);

    // Red: behind the green quad where they overlap
    Model* red = new Model(glm::vec3(-0.25f, 0.05f, 0.0f), glm::vec2(0.22f, 0.22f), quad);
    red->setColor(glm::vec4(0.86f, 0.28f, 0.24f, 1.0f));
    red->setLayer(0.0f);
    scene.addNode(red);

    // Green: higher layer, should cover the red overlap
    Model* green = new Model(glm::vec3(-0.08f, -0.02f, 0.3f), glm::vec2(0.22f, 0.22f), quad);
    green->setColor(glm::vec4(0.28f, 0.72f, 0.42f, 1.0f));
    green->setLayer(1.0f);
    scene.addNode(green);

    // Yellow: mid layer, off to the right
    Model* yellow = new Model(glm::vec3(0.38f, -0.12f, -0.2f), glm::vec2(0.18f, 0.28f), quad);
    yellow->setColor(glm::vec4(0.92f, 0.78f, 0.28f, 1.0f));
    yellow->setLayer(0.5f);
    scene.addNode(yellow);

    while (!context.shouldClose())
    {
        int framebufferWidth = 0;
        int framebufferHeight = 0;
        glfwGetFramebufferSize(context.getWindow(), &framebufferWidth, &framebufferHeight);
        glViewport(0, 0, framebufferWidth, framebufferHeight);
        camera.resize(framebufferWidth, framebufferHeight);

        glm::vec3 poseRed = red->getPose();
        poseRed.z += 0.01f;
        red->setPose(poseRed);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        scene.draw();
        context.swapBuffers();
        glfwPollEvents();
    }

    return 0;
}
