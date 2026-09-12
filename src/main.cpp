#include "engine/scene/Scene.h"
#include "engine/resource/ObjServer.h"
#include "engine/resource/TextureServer.h"
#include "engine/node/RigidBody.h"
#include "engine/physics/collision/ColliderPolygon2DMesh.h"
#include "engine/render/material/Material.h"
#include "engine/node/Model.h"
#include "engine/node/Model3d.h"
#include "engine/render/camera/Camera.h"
#include "engine/Engine.h"
#include "engine/resource/ShaderServer.h"
#include "engine/render/buffer/FrameBuffer.h"

int main()
{
    // create scene
    const int width = 800;
    const int height = 600;
    Engine engine(width, height);

    Texture* tx = new Texture(width / 2, height / 2, GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE);
    tx->setFilter(GL_NEAREST, GL_NEAREST);
    FrameBuffer* frameBuffer = new FrameBuffer();
    frameBuffer->setTexture(tx, tx->getWidth(), tx->getHeight());
    engine.setFBO(frameBuffer);

    Camera camera(width, height, 1.0f);
    Scene scene;
    scene.setCamera(&camera);
    engine.setScene(&scene);

    // load shaders
    ShaderServer::loadShader("uv", "shaders/default2d.vert", "shaders/uv.frag");
    ShaderServer::loadShader("bary2d", "shaders/default2d.vert", "shaders/bary.frag");
    ShaderServer::loadShader("bary3d", "shaders/default3d.vert", "shaders/bary.frag");
    Shader* uvShader = ShaderServer::getShader("uv");
    Shader* bary2d = ShaderServer::getShader("bary2d");
    Shader* bary3d = ShaderServer::getShader("bary3d");

    // load meshes
    ObjServer::loadMesh("quad", "resources/mesh/octagon.obj");
    ObjServer::loadMesh("cube", "resources/mesh/cube.obj");
    Mesh* quad = ObjServer::getMesh("quad");
    Mesh* cubeMesh = ObjServer::getMesh("cube");

    ColliderPolygon2DMeshServer::loadMesh("quad", quad->getVertices());

    // load materials
    TextureServer::loadTexture("fan", "resources/image/fan.png");
    Texture* white = TextureServer::getTexture("white");
    Texture* fan = TextureServer::getTexture("fan");

    Material solid(white);
    Material fanMaterial(fan, white);

    // create scene
    Model* backdrop = new Model(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec2(0.7f, 0.45f), quad, &solid, uvShader);
    backdrop->setColor(glm::vec4(0.22f, 0.24f, 0.32f, 1.0f));
    backdrop->setLayer(-1.0f);
    scene.addNode(backdrop);

    Model* red = new Model(glm::vec3(-0.25f, 0.05f, 0.0f), glm::vec2(0.22f, 0.22f), quad, &solid, bary2d);
    red->setColor(glm::vec4(0.86f, 0.28f, 0.24f, 1.0f));
    red->setLayer(2.0f);
    scene.addNode(red);

    Model* green = new Model(glm::vec3(-0.08f, -0.02f, 0.3f), glm::vec2(0.22f, 0.22f), quad, &solid, nullptr);
    green->setColor(glm::vec4(0.28f, 0.72f, 0.42f, 1.0f));
    green->setLayer(0.0f);
    scene.addNode(green);

    RigidBody* yellow = new RigidBody(
        glm::vec3(0.38f, -0.12f, -0.2f),
        glm::vec2(0.18f, 0.28f),
        quad,
        &solid,
        uvShader,
        Collider2D::polygon(ColliderPolygon2DMeshServer::getMesh("quad")),
        1.0f,
        glm::vec3(-0.1f, 0.0f, 0.0f)
    );
    yellow->setColor(glm::vec4(0.92f, 0.78f, 0.28f, 1.0f));
    yellow->setLayer(0.0f);
    scene.addNode(yellow);

    Model* fanModel = new Model(glm::vec3(0.12f, 0.28f, 0.0f), glm::vec2(0.2f, 0.2f), quad, &fanMaterial, nullptr);
    fanModel->setColor(glm::vec4(1.0f, 0.0f, 1.0f, 1.0f));
    fanModel->setLayer(1.5f);
    scene.addNode(fanModel);

    Model3d* cube = new Model3d(
        glm::vec3(0.28f, 0.08f, 1.0f),
        glm::angleAxis(glm::radians(45.0f), glm::vec3(0.5f, 0.5f, 0.0f)),
        glm::vec3(0.12f),
        cubeMesh,
        &solid,
        bary3d
    );
    cube->setColor(glm::vec4(0.85f, 0.55f, 0.25f, 1.0f));
    scene.addNode(cube);

    // collider mesh tracking points
    std::vector<Model*> trackingPoints;

    // main loop
    while (!engine.shouldClose())
    {
        glm::vec3 poseFan = fanModel->getPose();
        poseFan.z += 0.01f;
        fanModel->setPose(poseFan);

        glm::quat rotationCube = cube->getRotation();
        rotationCube = glm::angleAxis(glm::radians(1.0f), glm::vec3(0.0f, 0.0f, 1.0f)) * rotationCube;
        cube->setRotation(rotationCube);

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
            glm::vec3 pt = yellow->getModelMatrix() * glm::vec4(point, 0.0f, 1.0f);
            Model* trackingPoint = new Model(pt, glm::vec2(0.01f, 0.01f), quad, &solid, nullptr);
            float alpha = (float)i / (float)numPoints;
            trackingPoint->setColor(glm::vec4(1 - alpha, 0.0f, alpha, 1.0f));
            trackingPoint->setLayer(2.0f);
            scene.addNode(trackingPoint);
            trackingPoints.push_back(trackingPoint);
            i++;
        }

        engine.render();
        engine.present();
    }

    return 0;
}
