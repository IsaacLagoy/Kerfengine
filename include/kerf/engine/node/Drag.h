#pragma once

#include <kerf/engine/node/Button.h>


namespace kerf {

// forward declarations
class Drop;

// ------------------------------------------------
// Drag
// ------------------------------------------------

class Drag : public Button {
    friend class Drop;

private:
    glm::vec2 relativePosition; // world space
    glm::vec2 velocity; // world space, uses dt
    bool wasDragging = false;
    Drop* parkedDrop = nullptr;

    std::function<void(float)> onDropCallback;
    std::function<void(float)> onPickupCallback;

    bool locked = false;

public:
    Drag(const glm::vec3& pose, const glm::vec2& scale, Mesh* mesh, Material* material, Shader* shader, const Collider2D& collider);
    ~Drag();

    virtual void update(float dt, const glm::vec2& mousePosition, bool mouseDown, bool mousePressed) override;

    // getters
    const glm::vec2& getRelativePosition() const { return relativePosition; }
    const glm::vec2& getVelocity() const { return velocity; }

    // setters
    void setOnDropCallback(const std::function<void(float)>& callback);
    void setOnPickupCallback(const std::function<void(float)>& callback);

    void setLocked(bool locked) { this->locked = locked; }

protected:
    virtual void onDown(float dt) override;
    virtual void onUp(float dt) override;

    virtual void onDrop(float dt);
    virtual void onPickup(float dt);

private:
    bool isDragging() const;
};

} // namespace kerf