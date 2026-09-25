#pragma once

#include <kerf/engine/node/Drop.h>


namespace kerf {

/**
 * @brief Acts as a regular drop but doesn't release ownership of its drag until the drag is placed in another drop.
 * If a drag is taken out of a sticky drop and released outside of a drop, the drag is locked and pulled back to the sticky drop in pullTime seconds.
 * While the drag is returning, this drop stays locked so another drop cannot take it.
 * Once the drag has returned, both are unlocked.
 */
class StickyDrop : public Drop {
private:
    float pullTime = 1.0f; // > 0.0 so no div by 0
    bool lifted = false;
    bool returning = false;
    float pullElapsed = 0.0f;
    glm::vec2 pullFrom = glm::vec2(0.0f);

public:
    StickyDrop(
        const glm::vec3& pose,
        const glm::vec2& scale,
        Mesh* mesh,
        Material* material,
        Shader* shader,
        const Collider2D& collider,
        float pullTime
    );
    ~StickyDrop() override;

    virtual void update(float dt) override;
    virtual void updateButton(float dt, const glm::vec2& mousePosition, bool mouseDown, bool mousePressed) override;

    // getters
    float getPullTime() const;

    // setters
    void setPullTime(float pullTime);
    void setDrag(Drag* drag) override;

protected:
    void onDrop(float dt) override;
    void onPickup(float dt) override;
    bool shouldBeDown(bool wasDown, bool isHovered, bool mouseDown, bool mousePressed) const override;
};

} // namespace kerf
