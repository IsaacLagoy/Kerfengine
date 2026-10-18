#pragma once

#include <kerf/engine/node/drop/Drop.h>
#include <kerf/engine/node/drop/PullBack.h>


namespace kerf {

/**
 * @brief Acts as a regular drop but doesn't release ownership of its drag until the drag is placed in another drop.
 * If a drag is taken out of a sticky drop and released outside of a drop, the drag is locked and pulled back to the sticky drop in pullTime seconds.
 * While the drag is returning, it stays locked, so it can't be picked up or taken by another drop.
 * Picking a different drag does not wait for the return to finish.
 * Once the drag has returned, it is unlocked.
 */
class StickyDrop : public Drop {
private:
    PullBack pullBack;

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

    // can be set to true to induce returning animation
    void setLifted(bool lifted);

protected:
    void onDrop(float dt) override;
    void onPickup(float dt) override;
    bool shouldBeDown(bool wasDown, bool isHovered, bool mouseDown, bool mousePressed) const override;
};

} // namespace kerf
