#pragma once

#include "engine/node/Button.h"

class Drag;

class Drop : public Button {
private:
    Drag* drag = nullptr;

public:
    Drop(const glm::vec3& pose, const glm::vec2& scale, Mesh* mesh, Material* material, Shader* shader, const Collider2D& collider);
    ~Drop();

    virtual void update(float dt, const glm::vec2& mousePosition, bool mouseDown, bool mousePressed) override;

    void setDrag(Drag* drag);
    Drag* getDrag() const;

protected:
    virtual void onDown(float dt) override;
    virtual void onUp(float dt) override;
    virtual bool shouldBeDown(bool wasDown, bool isHovered, bool mouseDown, bool mousePressed) const override;
};