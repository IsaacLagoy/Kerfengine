#include <kerf/engine/scene/Scene.h>
#include <kerf/engine/node/Node.h>
#include <kerf/engine/node/RigidBody.h>
#include <kerf/engine/node/Button.h>
#include <kerf/engine/node/Drag.h>
#include <kerf/engine/render/camera/Camera.h>
#include <kerf/engine/input/Mouse.h>
#include <glm/glm.hpp>


namespace kerf {

Scene::Scene()
{
    // create root node
    root = new Node();
    root->scene = this;

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

    // create selected drag
    selectedDrag = nullptr;
}

Scene::~Scene()
{
    delete root;
    delete rigidBodyHead;
    delete rigidBodyTail;
    delete buttonHead;
    delete buttonTail;

    selectedDrag = nullptr;
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
    if (!node)
    {
        return;
    }

    root->addChild(node);
}

void Scene::removeNode(Node* node)
{
    if (!node || node == root)
    {
        return;
    }

    node->removeFromScene();
}

int Scene::getRigidBodyCount() const
{
    int count = 0;
    for (RigidBody* rigidBody = rigidBodyHead->nextRigidBody; rigidBody != rigidBodyTail; rigidBody = rigidBody->nextRigidBody)
    {
        ++count;
    }
    return count;
}

int Scene::getButtonCount() const
{
    int count = 0;
    for (Button* button = buttonHead->nextButton; button != buttonTail; button = button->nextButton)
    {
        ++count;
    }
    return count;
}

void Scene::setSelectedDrag(Drag* drag)
{
    selectedDrag = drag;
}

Drag* Scene::getSelectedDrag() const
{
    return selectedDrag;
}

Node* Scene::getRoot() const
{
    return root;
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

    // draw node tree
    root->draw(viewProjection);
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

    const glm::vec2 mousePosition = mouse.mouseWorld(*camera);
    const bool mouseDown = mouse.getLeftDown();
    const bool mousePressed = mouse.getLeftPressed();

    for (Button* button = buttonTail->prevButton; button != buttonHead; button = button->prevButton) 
    {
        button->update(dt, mousePosition, mouseDown, mousePressed);
    }

    // Drops claim selectedDrag in onUp; clear only after every button has updated.
    if (!mousePressed)
    {
        selectedDrag = nullptr;
    }
}

} // namespace kerf