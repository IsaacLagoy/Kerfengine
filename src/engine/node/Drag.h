#pragma once

#include "engine/node/Button.h"

class Drag : public Button {
private:
    glm::vec2 relativePosition; // world space
    glm::vec2 velocity; // world space, uses dt
    bool isDragging = false;
    bool wasDragging = false;

public:
    Drag(const glm::vec3& pose, const glm::vec2& scale, Mesh* mesh, Material* material, Shader* shader, const Collider2D& collider);
    ~Drag();

    virtual void update(float dt, const glm::vec2& mousePosition, bool mouseDown) override;

    // getters
    const glm::vec2& getRelativePosition() const { return relativePosition; }
    const glm::vec2& getVelocity() const { return velocity; }
    bool getIsDragging() const { return isDragging; }

protected:
    virtual void onDown(float dt) override;
    virtual void onUp(float dt) override;
};