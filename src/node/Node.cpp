#include "node/Node.h"


Node::Node() : Node(NodeType::NODE) {}

Node::Node(const glm::vec3& pose) : Node(pose, NodeType::NODE) {}

Node::Node(const glm::vec3& pose, const glm::vec2& scale) : Node(pose, scale, NodeType::NODE) {}

Node::Node(NodeType type) : type(type)
{
    computeModelMatrix(modelMatrix, pose, scale);
}

Node::Node(const glm::vec3& pose, NodeType type) : pose(pose), type(type)
{
    computeModelMatrix(modelMatrix, pose, scale);
}

Node::Node(const glm::vec3& pose, const glm::vec2& scale, NodeType type) :
    pose(pose),
    scale(scale),
    type(type)
{
    computeModelMatrix(modelMatrix, pose, scale);
}

Node::~Node() {}

void Node::setPose(const glm::vec3& pose)
{
    this->pose = pose;
    computeModelMatrix(modelMatrix, pose, scale);
}

void Node::setScale(const glm::vec2& scale)
{
    this->scale = scale;
    computeModelMatrix(modelMatrix, pose, scale);
}

glm::vec3 Node::getPose() const { return pose; }
glm::vec2 Node::getScale() const { return scale; }
glm::mat3 Node::getModelMatrix() const { return modelMatrix; }

void Node::computeModelMatrix(glm::mat3& modelMatrix, const glm::vec3& pose, const glm::vec2& scale)
{
    const float c = glm::cos(pose.z);
    const float s = glm::sin(pose.z);

    modelMatrix = glm::mat3(
        scale.x * c,  scale.x * s,  0.0f,
       -scale.y * s,  scale.y * c,  0.0f,
        pose.x,       pose.y,       1.0f
    );
}
