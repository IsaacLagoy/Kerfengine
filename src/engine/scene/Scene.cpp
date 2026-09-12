#include "engine/scene/Scene.h"
#include "engine/node/Node.h"
#include "engine/node/RigidBody.h"
#include "engine/node/Button.h"
#include "engine/render/camera/Camera.h"
#include "engine/input/Mouse.h"
#include <glm/glm.hpp>


Scene::Scene()
{
    // create node doubly-linked list
    head = new Node();
    tail = new Node();
    head->nextNode = tail;
    tail->prevNode = head;

    // create rigid body doubly-linked list
    rigidBodyHead = new RigidBody();
    rigidBodyTail = new RigidBody();
    rigidBodyHead->nextRigidBody = rigidBodyTail;
    rigidBodyTail->prevRigidBody = rigidBodyHead;

    // create button doubly-linked list
    buttonHead = new Button();
    buttonTail = new Button();
    buttonHead->nextButton = buttonTail;
    buttonTail->prevButton = buttonHead;
}

Scene::~Scene()
{
    while (head->nextNode != tail) 
    {
        removeNode(head->nextNode);
    }

    delete head;
    delete tail;
    delete rigidBodyHead;
    delete rigidBodyTail;
    delete buttonHead;
    delete buttonTail;
}

void Scene::setCamera(Camera* camera)
{
    this->camera = camera;
}

Camera* Scene::getCamera() const
{
    return camera;
}

// ------------------------------------------------
// node management
// ------------------------------------------------

void Scene::addNode(Node* node)
{
    node->insertNode(tail);

    if (auto* rigidBody = dynamic_cast<RigidBody*>(node)) 
    {
        rigidBody->insertRigidBody(rigidBodyTail);
    }

    if (auto* button = dynamic_cast<Button*>(node)) 
    {
        button->insertButton(buttonTail);
    }
}

void Scene::removeNode(Node* node)
{
    delete node;
}

RigidBody* Scene::getRigidBodyHead() const
{
    return rigidBodyHead;
}

RigidBody* Scene::getRigidBodyTail() const
{
    return rigidBodyTail;
}

Button* Scene::getButtonHead() const
{
    return buttonHead;
}

Button* Scene::getButtonTail() const
{
    return buttonTail;
}

// ------------------------------------------------
// rendering
// ------------------------------------------------

void Scene::draw() const
{
    // Camera once per frame; each Model/Text bind()s its own shader and
    // writes uViewProjection after that bind.
    glm::mat4 viewProjection(1.0f);
    if (camera) 
    {
        viewProjection = camera->getProjection() * camera->getView();
    }

    for (Node* node = head->nextNode; node != tail; node = node->nextNode) 
    {
        node->draw(viewProjection);
    }
}

// ------------------------------------------------
// updating
// ------------------------------------------------

void Scene::update(float dt, Mouse& mouse)
{
    for (RigidBody* rigidBody = rigidBodyHead->nextRigidBody; rigidBody != rigidBodyTail; rigidBody = rigidBody->nextRigidBody) 
    {
        rigidBody->update(dt);
    }

    for (Button* button = buttonHead->nextButton; button != buttonTail; button = button->nextButton) 
    {
        button->update(dt, mouse.mouseWorld(*camera), mouse.getLeftPressed());
    }
}