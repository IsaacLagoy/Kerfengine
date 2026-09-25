#pragma once

#include <cstddef>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>


namespace kerf {
    
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
    glm::mat4 localModelMatrix = glm::mat4(1.0f);
    glm::mat4 parentModelMatrix = glm::mat4(1.0f);
    glm::mat4 invModelMatrix = glm::mat4(1.0f);

    Node* parentNode = nullptr;
    std::vector<Node*> childNodes;

    // back pointer to scene
    Scene* scene = nullptr;

public:
    Node();
    virtual ~Node();

    // disable copying and moving
    Node(const Node& other) = delete;
    Node(Node&& other) = delete;
    Node& operator=(const Node& other) = delete;
    Node& operator=(Node&& other) = delete;

    void addChild(Node* child);
    void removeFromParent();

    Node* getParent() const;
    Scene* getScene() const;
    std::size_t getChildCount() const;
    Node* getChild(std::size_t index) const;

    // get the model matrix
    glm::mat4 getModelMatrix() const;
    glm::mat4 getInverseModelMatrix() const;

    // compute the model matrix
    static void computeModelMatrix(glm::mat4& modelMatrix, const glm::vec3& pose, const glm::vec2& scale);
    static void computeModelMatrix(glm::mat4& modelMatrix, const glm::vec3& position, const glm::quat& rotation, const glm::vec3& scale);

    void updateModelMatrix(const glm::vec3& pose, const glm::vec2& scale);
    void updateModelMatrix(const glm::vec3& position, const glm::quat& rotation, const glm::vec3& scale);

    glm::vec3 pointToLocalSpace(const glm::vec3& point) const;
    glm::vec2 pointToLocalSpace(const glm::vec2& point) const;

    // live
    virtual void update(float dt);

protected:
    virtual void draw(const glm::mat4& viewProjection);
    virtual void enterScene(Scene* scene);
    virtual void exitScene(Scene* scene);

private:
    void updateModelMatrix(const glm::mat4& parentModelMatrix);
    void unlinkTreeOnly();
    void removeFromScene();
    void propagateScene(Scene* oldScene, Scene* newScene);
    bool isSceneRoot() const;
    bool isDescendantOf(const Node* ancestor) const;
};

} // namespace kerf
