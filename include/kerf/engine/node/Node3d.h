#pragma once

#include <glm/glm.hpp>

#include <kerf/engine/node/Node.h>


namespace kerf {

class Node3d : public Node {
    friend class Scene;
    
private:
    glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::quat rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
    glm::vec3 scale = glm::vec3(1.0f, 1.0f, 1.0f);

public: 
    Node3d();
    Node3d(const glm::vec3& position, const glm::quat& rotation, const glm::vec3& scale);
    virtual ~Node3d();

    Node3d(const Node3d& other) = delete;
    Node3d(Node3d&& other) = delete;
    Node3d& operator=(const Node3d& other) = delete;
    Node3d& operator=(Node3d&& other) = delete;

    void setPosition(const glm::vec3& position);
    void setRotation(const glm::quat& rotation);
    void setScale(const glm::vec3& scale);

    glm::vec3 getPosition() const;
    glm::quat getRotation() const;
    glm::vec3 getScale() const;

    glm::vec3 getWorldPosition() const;
    glm::quat getWorldRotation() const;
    glm::vec3 getWorldScale() const;

protected:
    virtual void draw(const glm::mat4& viewProjection) override;
};

} // namespace kerf