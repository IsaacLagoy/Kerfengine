#include "engine/Engine.h"
#include "engine/node/Button.h"
#include "engine/node/Model.h"
#include "engine/node/Node.h"
#include "engine/node/RigidBody.h"
#include "engine/physics/collision/Collider2D.h"
#include "engine/render/camera/Camera.h"
#include "engine/render/material/Material.h"
#include "engine/resource/ObjServer.h"
#include "engine/resource/TextureServer.h"
#include "engine/scene/Scene.h"

#include <cstdlib>
#include <iostream>


#define CHECK(cond)                                                                 \
    do                                                                              \
    {                                                                               \
        if (!(cond))                                                                \
        {                                                                           \
            std::cerr << "CHECK failed: " #cond " (" << __FILE__ << ":" << __LINE__ \
                      << ")" << std::endl;                                          \
            std::abort();                                                           \
        }                                                                           \
    } while (0)

namespace {

RigidBody* makeBody(Mesh* mesh, Material* material, const glm::vec3& pose, const glm::vec3& velocity)
{
    return new RigidBody(
        pose,
        glm::vec2(0.1f, 0.1f),
        mesh,
        material,
        nullptr,
        Collider2D::box(1.0f, 1.0f),
        1.0f,
        velocity
    );
}

Button* makeButton(Mesh* mesh, Material* material, const glm::vec3& pose)
{
    return new Button(
        pose,
        glm::vec2(0.1f, 0.1f),
        mesh,
        material,
        nullptr,
        Collider2D::box(1.0f, 1.0f)
    );
}

void runHierarchyChecks(Engine& engine, Mesh* quad, Material* solid)
{
    Camera camera(800, 600, 1.0f);
    Mouse& mouse = engine.getMouse();

    {
        Node* parent = new Node();
        Node* child = new Node();
        Node* grandchild = new Node();

        parent->addChild(child);
        child->addChild(grandchild);

        CHECK(child->getParent() == parent);
        CHECK(grandchild->getParent() == child);
        CHECK(parent->getChildCount() == 1);
        CHECK(parent->getChild(0) == child);
        CHECK(parent->getScene() == nullptr);
        CHECK(child->getScene() == nullptr);
        CHECK(grandchild->getScene() == nullptr);

        child->addChild(parent);
        CHECK(child->getParent() == parent);
        CHECK(parent->getParent() == nullptr);

        delete parent;
    }

    {
        Scene scene;
        scene.setCamera(&camera);

        Node* wrapper = new Node();
        RigidBody* body = makeBody(quad, solid, glm::vec3(0.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        Button* button = makeButton(quad, solid, glm::vec3(0.2f, 0.0f, 0.0f));
        wrapper->addChild(body);
        wrapper->addChild(button);

        CHECK(wrapper->getScene() == nullptr);
        CHECK(body->getScene() == nullptr);
        CHECK(button->getScene() == nullptr);
        CHECK(scene.getRigidBodyCount() == 0);
        CHECK(scene.getButtonCount() == 0);

        scene.addNode(wrapper);

        CHECK(wrapper->getScene() == &scene);
        CHECK(body->getScene() == &scene);
        CHECK(button->getScene() == &scene);
        CHECK(wrapper->getParent() != nullptr);
        CHECK(body->getParent() == wrapper);
        CHECK(button->getParent() == wrapper);
        CHECK(scene.getRigidBodyCount() == 1);
        CHECK(scene.getButtonCount() == 1);

        glm::vec3 poseBefore = body->getPose();
        scene.update(0.016f, mouse);
        CHECK(body->getPose().x > poseBefore.x);

        scene.removeNode(wrapper);

        CHECK(wrapper->getParent() == nullptr);
        CHECK(wrapper->getScene() == nullptr);
        CHECK(body->getScene() == nullptr);
        CHECK(button->getScene() == nullptr);
        CHECK(body->getParent() == wrapper);
        CHECK(button->getParent() == wrapper);
        CHECK(scene.getRigidBodyCount() == 0);
        CHECK(scene.getButtonCount() == 0);

        glm::vec3 frozen = body->getPose();
        scene.update(0.016f, mouse);
        CHECK(body->getPose() == frozen);

        delete wrapper;
    }

    {
        Scene scene;
        scene.setCamera(&camera);

        Node* wrapper = new Node();
        RigidBody* body = makeBody(quad, solid, glm::vec3(0.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        Node* nested = new Node();
        wrapper->addChild(nested);
        nested->addChild(body);
        scene.addNode(wrapper);

        nested->removeFromParent();

        CHECK(nested->getParent() == wrapper->getParent());
        CHECK(nested->getScene() == &scene);
        CHECK(body->getScene() == &scene);
        CHECK(body->getParent() == nested);
        CHECK(wrapper->getChildCount() == 0);
        CHECK(scene.getRigidBodyCount() == 1);

        glm::vec3 poseBefore = body->getPose();
        scene.update(0.016f, mouse);
        CHECK(body->getPose().x > poseBefore.x);

        nested->removeFromParent();
        CHECK(nested->getParent() == wrapper->getParent());
        CHECK(nested->getScene() == &scene);
    }

    {
        Scene scene;
        scene.setCamera(&camera);

        Node* a = new Node();
        Node* b = new Node();
        Node* moved = new Node();
        Node* keptChild = new Node();
        moved->addChild(keptChild);

        scene.addNode(a);
        scene.addNode(b);
        a->addChild(moved);

        CHECK(moved->getParent() == a);
        CHECK(moved->getScene() == &scene);
        CHECK(keptChild->getScene() == &scene);
        CHECK(keptChild->getParent() == moved);

        b->addChild(moved);

        CHECK(moved->getParent() == b);
        CHECK(moved->getScene() == &scene);
        CHECK(keptChild->getParent() == moved);
        CHECK(keptChild->getScene() == &scene);
        CHECK(a->getChildCount() == 0);
        CHECK(b->getChildCount() == 1);
    }

    {
        Scene sceneA;
        Scene sceneB;
        sceneA.setCamera(&camera);
        sceneB.setCamera(&camera);

        Node* fromA = new Node();
        RigidBody* body = makeBody(quad, solid, glm::vec3(0.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        fromA->addChild(body);
        sceneA.addNode(fromA);

        Node* inB = new Node();
        sceneB.addNode(inB);

        CHECK(fromA->getScene() == &sceneA);
        CHECK(body->getScene() == &sceneA);
        CHECK(sceneA.getRigidBodyCount() == 1);
        CHECK(sceneB.getRigidBodyCount() == 0);

        inB->addChild(fromA);

        CHECK(fromA->getParent() == inB);
        CHECK(fromA->getScene() == &sceneB);
        CHECK(body->getScene() == &sceneB);
        CHECK(body->getParent() == fromA);
        CHECK(sceneA.getRigidBodyCount() == 0);
        CHECK(sceneB.getRigidBodyCount() == 1);

        glm::vec3 poseBefore = body->getPose();
        sceneA.update(0.016f, mouse);
        CHECK(body->getPose() == poseBefore);
        sceneB.update(0.016f, mouse);
        CHECK(body->getPose().x > poseBefore.x);
    }

    {
        Scene scene;
        scene.setCamera(&camera);

        Node* parent = new Node();
        Node* siblingKeep = new Node();
        Node* mid = new Node();
        RigidBody* doomed = makeBody(quad, solid, glm::vec3(0.0f), glm::vec3(0.0f));
        mid->addChild(doomed);
        parent->addChild(siblingKeep);
        parent->addChild(mid);
        scene.addNode(parent);

        CHECK(scene.getRigidBodyCount() == 1);
        CHECK(parent->getChildCount() == 2);

        delete mid;

        CHECK(parent->getScene() == &scene);
        CHECK(siblingKeep->getParent() == parent);
        CHECK(siblingKeep->getScene() == &scene);
        CHECK(parent->getChildCount() == 1);
        CHECK(parent->getChild(0) == siblingKeep);
        CHECK(scene.getRigidBodyCount() == 0);

        scene.update(0.016f, mouse);
        scene.draw();
    }

    std::cout << "tree hierarchy checks passed" << std::endl;
}

} // namespace

int main()
{
    const int width = 800;
    const int height = 600;
    Engine engine(width, height);

    Camera camera(width, height, 1.0f);
    Scene scene;
    scene.setCamera(&camera);

    ObjServer::loadMesh("quad", "resources/mesh/quad.obj");
    Mesh* quad = ObjServer::getMesh("quad");

    Texture* white = TextureServer::getTexture("white");
    Material solid(white);

    runHierarchyChecks(engine, quad, &solid);

    Model* parent = new Model(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec2(0.35f, 0.35f), quad, &solid, nullptr);
    parent->setColor(glm::vec4(0.35f, 0.55f, 0.85f, 1.0f));
    parent->setLayer(0.0f);

    Model* child = new Model(glm::vec3(0.7f, 0.0f, 0.0f), glm::vec2(0.18f, 0.18f), quad, &solid, nullptr);
    child->setColor(glm::vec4(0.90f, 0.45f, 0.25f, 1.0f));
    child->setLayer(1.0f);
    parent->addChild(child);

    scene.addNode(parent);
    engine.setScene(&scene);

    while (!engine.shouldClose())
    {
        glm::vec3 pose = parent->getPose();
        pose.z += 0.02f;
        parent->setPose(pose);

        engine.update();
        engine.render();
    }

    return 0;
}
