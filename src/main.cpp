#include "engine/scene/Scene.h"
#include "engine/resource/ObjServer.h"
#include "engine/resource/ShaderServer.h"
#include "engine/resource/TextureServer.h"
#include "engine/node/RigidBody.h"
#include "engine/physics/collision/ColliderPolygon2DMesh.h"
#include "engine/render/material/Material.h"
#include "engine/node/Model.h"
#include "engine/render/camera/Camera.h"
#include "engine/Engine.h"

int main()
{
    // create scene
    const int width = 800;
    const int height = 600;
    Engine engine(width, height);

    Camera camera(width, height, 1.0f);
    Scene scene;
    scene.setCamera(&camera);
    engine.setScene(&scene);

    // load meshes
    ObjServer::loadMesh("quad", "resources/mesh/octagon.obj");
    Mesh* quad = ObjServer::getMesh("quad");

    ColliderPolygon2DMeshServer::loadMesh("quad", quad->getVertices());

    // load shaders
    ShaderServer::loadShader("default2d", "shaders/default2d.vert", "shaders/default2d.frag");

    // load materials
    TextureServer::loadTexture("white", "resources/image/white.png");
    TextureServer::loadTexture("fan", "resources/image/fan.png");
    Texture* white = TextureServer::getTexture("white");
    Texture* fan = TextureServer::getTexture("fan");

    Material solid(white);
    Material fanMaterial(fan, white);

    // create scene
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

    RigidBody* yellow = new RigidBody(
        glm::vec3(0.38f, -0.12f, -0.2f),
        glm::vec2(0.18f, 0.28f),
        quad,
        &solid,
        Collider2D::polygon(ColliderPolygon2DMeshServer::getMesh("quad")),
        1.0f,
        glm::vec3(-1.0f, 0.0f, 0.0f)
    );
    yellow->setColor(glm::vec4(0.92f, 0.78f, 0.28f, 1.0f));
    yellow->setLayer(0.5f);
    scene.addNode(yellow);

    Model* fanModel = new Model(glm::vec3(0.12f, 0.28f, 0.0f), glm::vec2(0.2f, 0.2f), quad, &fanMaterial);
    fanModel->setColor(glm::vec4(1.0f, 0.0f, 1.0f, 1.0f));
    fanModel->setLayer(1.5f);
    scene.addNode(fanModel);

    // collider mesh tracking points
    std::vector<Model*> trackingPoints;

    // main loop
    while (!engine.shouldClose())
    {
        glm::vec3 poseFan = fanModel->getPose();
        poseFan.z += 0.01f;
        fanModel->setPose(poseFan);

        engine.update();

        // remove old tracking points
        for (const auto& point : trackingPoints) {
            scene.removeNode(point);
        }
        trackingPoints.clear();

        // update collider mesh tracking points
        int numPoints = yellow->getCollider().getMesh()->getVertices().size();
        int i = 0;
        for (const auto& point : yellow->getCollider().getMesh()->getVertices()) {
            glm::vec3 pt = yellow->getModelMatrix() * glm::vec3(point, 1.0f);
            Model* trackingPoint = new Model(pt, glm::vec2(0.01f, 0.01f), quad, &solid);
            float alpha = (float)i / (float)numPoints;
            trackingPoint->setColor(glm::vec4(1 - alpha, 0.0f, alpha, 1.0f));
            trackingPoint->setLayer(2.0f);
            scene.addNode(trackingPoint);
            trackingPoints.push_back(trackingPoint);
            i++;
        }

        // engine stuff
        engine.render();
    }

    return 0;
}
