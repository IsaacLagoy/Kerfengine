#include "engine/node/Node.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>


Node::Node() : modelMatrix(glm::mat4(1.0f)) {}

Node::~Node() 
{
    unlinkNode();
}

glm::mat4 Node::getModelMatrix() const 
{ 
    return modelMatrix; 
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

void Node::insertNode(Node* pos)
{
    this->nextNode = pos;
    this->prevNode = pos->prevNode;
    pos->prevNode->nextNode = this;
    pos->prevNode = this;
}

void Node::unlinkNode()
{
    if (prevNode) {
        prevNode->nextNode = nextNode;
    }

    if (nextNode) {
        nextNode->prevNode = prevNode;
    }
    
    nextNode = nullptr;
    prevNode = nullptr;
}

void Node::draw(const glm::mat4& viewProjection)
{
    (void) viewProjection;
    // no op
}

glm::mat4& Node::modelMatrixRef() 
{ 
    return modelMatrix; 
}