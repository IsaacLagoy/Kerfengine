#pragma once

#include "engine/node/Model.h"
#include "engine/physics/collision/Collider2D.h"

#include <functional>

// forward declarations
class Scene;
class Shader;

class Button : public Model {
    friend class Scene;

private:
    Collider2D collider;

    std::function<void(float dt)> onDownCallback;
    std::function<void(float dt)> onUpCallback;
    std::function<void(float dt)> onPressedCallback;
    std::function<void(float dt)> onReleasedCallback;
    std::function<void(float dt)> onHoverCallback;
    std::function<void(float dt)> onLeaveCallback;

    bool isDown = false;
    bool isHovered = false;

    Button* nextButton = nullptr;
    Button* prevButton = nullptr;

public:
    Button(const glm::vec3& pose, const glm::vec2& scale, Mesh* mesh, Material* material, Shader* shader, const Collider2D& collider);
    ~Button();

    void update(float dt, const glm::vec2& mousePosition, bool mouseDown);

    const Collider2D& getCollider() const { return collider; }
    bool getIsDown() const { return isDown; }
    bool getIsHovered() const { return isHovered; }

    // ------------------------------------------------------------
    // Event callbacks
    // ------------------------------------------------------------

    void setOnDownCallback(std::function<void(float dt)> callback) { onDownCallback = callback; }
    void setOnUpCallback(std::function<void(float dt)> callback) { onUpCallback = callback; }
    void setOnPressedCallback(std::function<void(float dt)> callback) { onPressedCallback = callback; }
    void setOnReleasedCallback(std::function<void(float dt)> callback) { onReleasedCallback = callback; }
    void setOnHoverCallback(std::function<void(float dt)> callback) { onHoverCallback = callback; }
    void setOnLeaveCallback(std::function<void(float dt)> callback) { onLeaveCallback = callback; }

protected:
    // protected constructor for scene sentinel nodes
    Button();

    virtual void draw(const glm::mat4& viewProjection) override;

    virtual void onDown(float dt);
    virtual void onUp(float dt);
    virtual void onPressed(float dt);
    virtual void onReleased(float dt);
    virtual void onHover(float dt);
    virtual void onLeave(float dt);

private:
    void insertButton(Button* pos);
    void unlinkButton();
    
};