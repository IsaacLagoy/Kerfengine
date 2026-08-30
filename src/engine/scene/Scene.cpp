#include "engine/scene/Scene.h"
#include "engine/node/Model.h"
#include "engine/node/RigidBody.h"
#include "engine/render/camera/Camera.h"
#include "engine/resource/ShaderServer.h"

#include <glm/gtc/type_ptr.hpp>


Scene::Scene()
{
    // create node doubly-linked list
    head = new Node();
    tail = new Node();
    head->nextNode = tail;
    tail->prevNode = head;

    // create model doubly-linked list
    modelHead = new Model();
    modelTail = new Model();
    modelHead->nextModel = modelTail;
    modelTail->prevModel = modelHead;

    // create rigid body doubly-linked list
    rigidBodyHead = new RigidBody();
    rigidBodyTail = new RigidBody();
    rigidBodyHead->nextRigidBody = rigidBodyTail;
    rigidBodyTail->prevRigidBody = rigidBodyHead;
}

Scene::~Scene()
{
    while (head->nextNode != tail) {
        removeNode(head->nextNode);
    }

    delete head;
    delete tail;
    delete modelHead;
    delete modelTail;
    delete rigidBodyHead;
    delete rigidBodyTail;
}

void Scene::addNode(Node* node)
{
    node->insertNode(tail);

    if (node->type == NodeType::MODEL || node->type == NodeType::RIGID_BODY) {
        static_cast<Model*>(node)->insertModel(modelTail);
    }

    if (node->type == NodeType::RIGID_BODY) {
        static_cast<RigidBody*>(node)->insertRigidBody(rigidBodyTail);
    }
}

void Scene::removeNode(Node* node)
{
    delete node;
}

void Scene::setCamera(Camera* camera)
{
    this->camera = camera;
}

Camera* Scene::getCamera() const
{
    return camera;
}

RigidBody* Scene::getRigidBodyHead() const
{
    return rigidBodyHead;
}

RigidBody* Scene::getRigidBodyTail() const
{
    return rigidBodyTail;
}

// ------------------------------------------------
// rendering
// ------------------------------------------------

void Scene::draw() const
{
    // bind default shader
    Shader* shader = ShaderServer::getShader("default2d");
    shader->bind();

    // set view-proj matrix
    glm::mat4 viewProjection(1.0f);
    if (camera) {
        viewProjection = camera->getProjection() * camera->getView();
    }
    glUniformMatrix4fv(shader->getUniformLocation("uViewProjection"), 1, GL_FALSE, glm::value_ptr(viewProjection));

    // draw all models
    for (Model* model = modelHead->nextModel; model != modelTail; model = model->nextModel) {
        model->draw();
    }
}

// ------------------------------------------------
// updating
// ------------------------------------------------

void Scene::update(float dt)
{
    for (RigidBody* rigidBody = rigidBodyHead->nextRigidBody; rigidBody != rigidBodyTail; rigidBody = rigidBody->nextRigidBody) {
        rigidBody->update(dt);
    }
}