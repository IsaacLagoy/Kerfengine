#include "scene/Scene.h"
#include "node/Model.h"
#include "render/camera/Camera.h"
#include "resource/ShaderServer.h"

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
}

void Scene::addNode(Node* node)
{
    insertNode(node, tail);

    if (node->type == NodeType::MODEL) {
        insertModel(static_cast<Model*>(node), modelTail);
    }
}

void Scene::removeNode(Node* node)
{
    if (node->type == NodeType::MODEL) {
        unlinkModel(static_cast<Model*>(node));
    }

    unlinkNode(node);
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
// node data structure helpers
// ------------------------------------------------

void Scene::insertNode(Node* node, Node* pos)
{
    node->nextNode = pos;
    node->prevNode = pos->prevNode;
    pos->prevNode->nextNode = node;
    pos->prevNode = node;
}

void Scene::unlinkNode(Node* node)
{
    node->prevNode->nextNode = node->nextNode;
    node->nextNode->prevNode = node->prevNode;
    node->nextNode = nullptr;
    node->prevNode = nullptr;
}

void Scene::insertModel(Model* model, Model* pos)
{
    model->nextModel = pos;
    model->prevModel = pos->prevModel;
    pos->prevModel->nextModel = model;
    pos->prevModel = model;
}

void Scene::unlinkModel(Model* model)
{
    model->prevModel->nextModel = model->nextModel;
    model->nextModel->prevModel = model->prevModel;
    model->nextModel = nullptr;
    model->prevModel = nullptr;
}