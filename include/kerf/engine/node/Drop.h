#pragma once

#include <kerf/engine/node/Button.h>


namespace kerf {

// forward declarations
class Drag;

class Drop : public Button {
    friend class Drag;

private:
    Drag* drag = nullptr;
    bool locked = false;

    std::function<void(float)> onDropCallback;
    std::function<void(float)> onPickupCallback;

public:
    Drop(const glm::vec3& pose, const glm::vec2& scale, Mesh* mesh, Material* material, Shader* shader, const Collider2D& collider);
    ~Drop();

    virtual void updateButton(float dt, const glm::vec2& mousePosition, bool mouseDown, bool mousePressed) override;

    // getters
    Drag* getDrag() const;
    bool isLocked() const;

    // setters
    virtual void setDrag(Drag* drag);
    void setLocked(bool locked);
    void setOnDropCallback(const std::function<void(float)>& callback);
    void setOnPickupCallback(const std::function<void(float)>& callback);

protected:
    virtual void onDown(float dt) override;
    virtual void onUp(float dt) override;
    virtual void onDrop(float dt);
    virtual void onPickup(float dt);

    void assignParkedDrop(Drag* drag);
    virtual void forgetDrag(Drag* leaving);

    virtual bool shouldBeDown(bool wasDown, bool isHovered, bool mouseDown, bool mousePressed) const override;
};

} // namespace kerf