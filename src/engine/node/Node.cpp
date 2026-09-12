#include "engine/node/Node.h"
#include "engine/scene/Scene.h"

#include <algorithm>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>


Node::Node() : modelMatrix(glm::mat4(1.0f)) {}

Node::~Node() 
{
    unlinkTreeOnly();
    scene = nullptr;

    std::vector<Node*> children = std::move(childNodes);
    childNodes.clear();
    for (Node* child : children)
    {
        child->parentNode = nullptr;
        delete child;
    }
}

void Node::addChild(Node* child)
{
    if (!child || child == this || child->isSceneRoot())
    {
        return;
    }

    if (isDescendantOf(child))
    {
        return;
    }

    if (child->parentNode == this)
    {
        return;
    }

    Scene* oldScene = child->scene;
    Scene* newScene = scene;

    child->unlinkTreeOnly();
    childNodes.push_back(child);
    child->parentNode = this;
    child->updateModelMatrix(modelMatrix);

    if (oldScene != newScene)
    {
        child->propagateScene(oldScene, newScene);
    }
}

void Node::removeFromParent()
{
    if (!parentNode || isSceneRoot())
    {
        return;
    }

    if (scene)
    {
        Node* root = scene->getRoot();
        if (parentNode == root)
        {
            return;
        }

        root->addChild(this);
        return;
    }

    unlinkTreeOnly();
    updateModelMatrix(glm::mat4(1.0f));
}

void Node::removeFromScene()
{
    if (isSceneRoot())
    {
        return;
    }

    Scene* oldScene = scene;
    unlinkTreeOnly();
    updateModelMatrix(glm::mat4(1.0f));

    if (oldScene)
    {
        propagateScene(oldScene, nullptr);
    }
}

Node* Node::getParent() const
{
    return parentNode;
}

Scene* Node::getScene() const
{
    return scene;
}

std::size_t Node::getChildCount() const
{
    return childNodes.size();
}

Node* Node::getChild(std::size_t index) const
{
    if (index >= childNodes.size())
    {
        return nullptr;
    }

    return childNodes[index];
}

glm::mat4 Node::getModelMatrix() const 
{ 
    return modelMatrix; 
}

glm::mat4 Node::getInverseModelMatrix() const
{
    return invModelMatrix;
}

void Node::computeModelMatrix(glm::mat4& modelMatrix, const glm::vec3& pose, const glm::vec2& scale)
{
    modelMatrix = glm::mat4(1.0f);
    modelMatrix = glm::translate(modelMatrix, glm::vec3(pose.x, pose.y, 0.0f));
    modelMatrix = glm::rotate(modelMatrix, pose.z, glm::vec3(0.0f, 0.0f, 1.0f));
    modelMatrix = glm::scale(modelMatrix, glm::vec3(scale.x, scale.y, 1.0f));   
}

void Node::computeModelMatrix(glm::mat4& modelMatrix, const glm::vec3& position, const glm::quat& rotation, const glm::vec3& scale)
{
    modelMatrix = glm::mat4(1.0f);
    modelMatrix = glm::translate(modelMatrix, position);
    modelMatrix = modelMatrix * glm::mat4_cast(rotation);
    modelMatrix = glm::scale(modelMatrix, scale);
}

glm::vec3 Node::pointToLocalSpace(const glm::vec3& point) const
{
    return glm::vec3(glm::inverse(modelMatrix) * glm::vec4(point, 1.0f));
}

glm::vec2 Node::pointToLocalSpace(const glm::vec2& point) const
{
    return glm::vec2(glm::inverse(modelMatrix) * glm::vec4(point, 0.0f, 1.0f));
}

void Node::draw(const glm::mat4& viewProjection)
{    
    for (Node* child : childNodes)
    {
        child->draw(viewProjection);
    }
}

void Node::enterScene(Scene* /*scene*/)
{
}

void Node::exitScene(Scene* /*scene*/)
{
}

void Node::unlinkTreeOnly()
{
    if (!parentNode) 
    {
        return;
    }

    auto& siblings = parentNode->childNodes;
    siblings.erase(std::remove(siblings.begin(), siblings.end(), this), siblings.end());
    parentNode = nullptr;
}

void Node::propagateScene(Scene* oldScene, Scene* newScene)
{
    if (oldScene)
    {
        exitScene(oldScene);
    }

    scene = newScene;

    if (newScene)
    {
        enterScene(newScene);
    }

    for (Node* child : childNodes)
    {
        child->propagateScene(oldScene, newScene);
    }
}

bool Node::isSceneRoot() const
{
    return scene && scene->getRoot() == this;
}

bool Node::isDescendantOf(const Node* ancestor) const
{
    if (!ancestor)
    {
        return false;
    }

    for (const Node* node = parentNode; node; node = node->parentNode)
    {
        if (node == ancestor)
        {
            return true;
        }
    }

    return false;
}

// ------------------------------------------------------------
// Update Model Matrix
// ------------------------------------------------------------

void Node::updateModelMatrix(const glm::vec3& pose, const glm::vec2& scale)
{
    computeModelMatrix(localModelMatrix, pose, scale);
    updateModelMatrix(parentModelMatrix);
}

void Node::updateModelMatrix(const glm::vec3& position, const glm::quat& rotation, const glm::vec3& scale)
{
    computeModelMatrix(localModelMatrix, position, rotation, scale);
    updateModelMatrix(parentModelMatrix);
}

void Node::updateModelMatrix(const glm::mat4& parentModelMatrix)
{
    this->parentModelMatrix = parentModelMatrix;
    modelMatrix = parentModelMatrix * localModelMatrix;
    invModelMatrix = glm::inverse(modelMatrix);

    // update children's model matrix
    for (Node* child : childNodes)
    {
        child->updateModelMatrix(modelMatrix);
    }
}
