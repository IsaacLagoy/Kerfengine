#pragma once

#include <glm/glm.hpp>

#include <kerf/engine/node/Node.h>

namespace kerf {

class Node2d : public Node {
    friend class Scene;
    
private:
    // x (right), y (up), r (rads)
    glm::vec3 pose = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec2 scale = glm::vec2(1.0f, 1.0f);

public:
    Node2d();
    Node2d(const glm::vec3& pose);
    Node2d(const glm::vec3& pose, const glm::vec2& scale);
    virtual ~Node2d();

    Node2d(const Node2d& other) = delete;
    Node2d(Node2d&& other) = delete;
    Node2d& operator=(const Node2d& other) = delete;
    Node2d& operator=(Node2d&& other) = delete;

    void setPose(const glm::vec3& pose);
    void setScale(const glm::vec2& scale);

    glm::vec3 getPose() const;
    glm::vec2 getScale() const;

protected:
    virtual void draw(const glm::mat4& viewProjection) override;
};

} // namespace kerf