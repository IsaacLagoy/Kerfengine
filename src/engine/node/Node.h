#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

// TODO add inheritance

// forward declarations
class Scene;

/**
 * @brief Base class for all nodes in the scene
 *
 * keeps the internal structure for all nodes, 2d and 3d
 * controls the model matrix and stack of nodes
 * 
 */
class Node {
    friend class Scene;

private:
    glm::mat4 modelMatrix = glm::mat4(1.0f);

    Node* nextNode = nullptr;
    Node* prevNode = nullptr;

public:
    Node();
    virtual ~Node();

    // disable copying and moving
    Node(const Node& other) = delete;
    Node(Node&& other) = delete;
    Node& operator=(const Node& other) = delete;
    Node& operator=(Node&& other) = delete;

    // get the model matrix
    glm::mat4 getModelMatrix() const;

    // compute the model matrix
    static void computeModelMatrix(glm::mat4& modelMatrix, const glm::vec3& pose, const glm::vec2& scale);
    static void computeModelMatrix(glm::mat4& modelMatrix, const glm::vec3& position, const glm::quat& rotation, const glm::vec3& scale);

protected:
    void insertNode(Node* pos);
    void unlinkNode();
    virtual void draw(const glm::mat4& viewProjection);
    glm::mat4& modelMatrixRef();    
};
