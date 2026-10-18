#pragma once

#include <kerf/engine/node/drop/DropAccordion.h>
#include <kerf/engine/node/drop/PullBack.h>

#include <unordered_map>


namespace kerf {

/**
 * @brief A DropAccordion whose cards follow the StickyDrop rules.
 *
 * The row keeps ownership of a card until another target claims it. A card that is
 * picked up keeps its slot: at the cursor while over the row, at its home slot
 * otherwise. Released outside any target, it is locked and pulled back to that slot
 * in pullTime seconds, then unlocked. While returning it can't be picked up or
 * claimed by another target. Several cards can return at once, and picking a
 * different card does not wait for a return to finish.
 */
class StickyDropAccordion : public DropAccordion {
private:
    float pullTime;
    std::unordered_map<const Drag*, PullBack> pulls; // one per card in the row

public:
    StickyDropAccordion(
        const glm::vec3& pose,
        const glm::vec2& scale,
        Mesh* mesh,
        Material* material,
        Shader* shader,
        const Collider2D& collider,
        float pullTime,
        float followSpeed = 14.0f
    );
    ~StickyDropAccordion() override;

    virtual void update(float dt) override;

    // getters
    float getPullTime() const;
    bool isReturning(const Drag* drag) const;

    // setters
    void setPullTime(float pullTime);

    // Like StickyDrop::setLifted, for one card in this row: true starts a return to its
    // slot (unless one is running), false cancels a return.
    void setLifted(Drag* drag, bool lifted);

protected:
    bool keepsCarried() const override;
    glm::vec2 moveItem(Drag* drag, const glm::vec2& pos, const glm::vec2& target, float dt) override;
    void forgetDrag(Drag* leaving) override;

private:
    PullBack& pullFor(const Drag* drag);
};

} // namespace kerf
