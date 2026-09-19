#pragma once

#include "engine/node/Button.h"

// forward declarations
class Drop;

class Drag : public Button {
    friend class Drop;

private:
    glm::vec2 relativePosition; // world space
    glm::vec2 velocity; // world space, uses dt
    bool wasDragging = false;
    Drop* parkedDrop = nullptr;

public:
    Drag(const glm::vec3& pose, const glm::vec2& scale, Mesh* mesh, Material* material, Shader* shader, const Collider2D& collider);
    ~Drag();

    virtual void update(float dt, const glm::vec2& mousePosition, bool mouseDown, bool mousePressed) override;

    // getters
    const glm::vec2& getRelativePosition() const { return relativePosition; }
    const glm::vec2& getVelocity() const { return velocity; }

protected:
    virtual void onDown(float dt) override;
    virtual void onUp(float dt) override;

private:
    bool isDragging() const;
};