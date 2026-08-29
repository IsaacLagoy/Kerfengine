#pragma once

#include <glm/glm.hpp>

// TODO add inheritance

// forward declarations
class Scene;

enum class NodeType {
    INVALID,
    NODE,
    MODEL,
    LIGHT,
};

class Node {

friend class Scene;

private:
    glm::mat3 modelMatrix = glm::mat3(1.0f);

    // x (right), y (up), r (rads)
    glm::vec3 pose = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec2 scale = glm::vec2(1.0f, 1.0f);

    Node* nextNode = nullptr;
    Node* prevNode = nullptr;

    NodeType type = NodeType::INVALID;

public:
    Node();
    Node(const glm::vec3& pose);
    Node(const glm::vec3& pose, const glm::vec2& scale);
    virtual ~Node();

    Node(const Node& other) = delete;
    Node(Node&& other) = delete;
    Node& operator=(const Node& other) = delete;
    Node& operator=(Node&& other) = delete;

    void setPose(const glm::vec3& pose);

    void setX(float x) { setPose(glm::vec3(x, pose.y, pose.z)); }
    void setY(float y) { setPose(glm::vec3(pose.x, y, pose.z)); }
    void setR(float r) { setPose(glm::vec3(pose.x, pose.y, r)); }

    void setScale(const glm::vec2& scale);

    void setScaleX(float x) { setScale(glm::vec2(x, scale.y)); }
    void setScaleY(float y) { setScale(glm::vec2(scale.x, y)); }

    glm::vec3 getPose() const;
    glm::vec2 getScale() const;
    glm::mat3 getModelMatrix() const;

protected:
    explicit Node(NodeType type);
    Node(const glm::vec3& pose, NodeType type);
    Node(const glm::vec3& pose, const glm::vec2& scale, NodeType type);

private:
    static void computeModelMatrix(glm::mat3& modelMatrix, const glm::vec3& pose, const glm::vec2& scale);
};
