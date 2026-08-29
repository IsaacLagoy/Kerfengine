#include "engine/scene/Scene.h"
#include "engine/resource/ObjServer.h"
#include "engine/resource/ShaderServer.h"
#include "engine/resource/TextureServer.h"
#include "engine/render/material/Material.h"
#include "engine/node/Model.h"
#include "engine/render/camera/Camera.h"
#include "engine/Engine.h"

int main()
{
    const int width = 800;
    const int height = 600;
    Engine engine(800, 600);

    Camera camera(width, height, 1.0f);
    Scene scene;
    scene.setCamera(&camera);
    engine.setScene(&scene);

    ObjServer::loadMesh("quad", "resources/mesh/quad.obj");
    Mesh* quad = ObjServer::getMesh("quad");

    ShaderServer::loadShader("default2d", "shaders/default2d.vert", "shaders/default2d.frag");

    TextureServer::loadTexture("white", "resources/image/white.png");
    TextureServer::loadTexture("fan", "resources/image/fan.png");
    Texture* white = TextureServer::getTexture("white");
    Texture* fan = TextureServer::getTexture("fan");

    Material solid(white);
    Material fanMaterial(fan, white);

    Model* backdrop = new Model(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec2(0.7f, 0.45f), quad, &solid);
    backdrop->setColor(glm::vec4(0.22f, 0.24f, 0.32f, 1.0f));
    backdrop->setLayer(-1.0f);
    scene.addNode(backdrop);

    Model* red = new Model(glm::vec3(-0.25f, 0.05f, 0.0f), glm::vec2(0.22f, 0.22f), quad, &solid);
    red->setColor(glm::vec4(0.86f, 0.28f, 0.24f, 1.0f));
    red->setLayer(0.0f);
    scene.addNode(red);

    Model* green = new Model(glm::vec3(-0.08f, -0.02f, 0.3f), glm::vec2(0.22f, 0.22f), quad, &solid);
    green->setColor(glm::vec4(0.28f, 0.72f, 0.42f, 1.0f));
    green->setLayer(1.0f);
    scene.addNode(green);

    Model* yellow = new Model(glm::vec3(0.38f, -0.12f, -0.2f), glm::vec2(0.18f, 0.28f), quad, &solid);
    yellow->setColor(glm::vec4(0.92f, 0.78f, 0.28f, 1.0f));
    yellow->setLayer(0.5f);
    scene.addNode(yellow);

    Model* fanModel = new Model(glm::vec3(0.12f, 0.28f, 0.0f), glm::vec2(0.2f, 0.2f), quad, &fanMaterial);
    fanModel->setColor(glm::vec4(1.0f, 0.0f, 1.0f, 1.0f));
    fanModel->setLayer(1.5f);
    scene.addNode(fanModel);

    while (!engine.shouldClose())
    {
        glm::vec3 poseFan = fanModel->getPose();
        poseFan.z += 0.01f;
        fanModel->setPose(poseFan);

        engine.update();
        engine.render();
    }

    return 0;
}
