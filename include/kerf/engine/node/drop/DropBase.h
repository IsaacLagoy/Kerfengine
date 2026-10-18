#pragma once

#include <kerf/engine/node/Button.h>


namespace kerf {

// forward declarations
class Drag;

/**
 * @brief Slot-agnostic half of a drop target.
 *
 * Owns the protocol between a Drag and anything that can hold one: the lock flag,
 * the drop/pickup callbacks, the "held drag claims a hovered target on release"
 * flow, and the Drag::parkedDrop back-pointer bookkeeping.
 *
 * How many drags a target holds, where they sit, and when it accepts another is left
 * to subclasses through canAccept, accept, and forgetDrag.
 *
 * Drag befriends this class, not its subclasses (friendship is not inherited), so
 * subclasses reach the Drag internals through the protected helpers below.
 */
class DropBase : public Button {
    friend class Drag;

private:
    bool locked = false;

    std::function<void(float)> onDropCallback;
    std::function<void(float)> onPickupCallback;

public:
    DropBase(const glm::vec3& pose, const glm::vec2& scale, Mesh* mesh, Material* material, Shader* shader, const Collider2D& collider);

    // getters
    bool isLocked() const;

    // setters
    void setLocked(bool locked);
    void setOnDropCallback(const std::function<void(float)>& callback);
    void setOnPickupCallback(const std::function<void(float)>& callback);

protected:
    // ------------------------------------------------------------
    // Slot policy, implemented by subclasses
    // ------------------------------------------------------------

    // Whether this target would take incoming right now. Lock checks are done by the
    // base, so this only answers for the target's own capacity rules.
    virtual bool canAccept(const Drag* incoming) const = 0;

    // Store incoming and move it into place. The base has already set
    // incoming's parkedDrop to this target.
    virtual void accept(Drag* incoming) = 0;

    // Silently drop any reference to leaving (no callbacks).
    // Also called by ~Drag and when another target claims leaving.
    virtual void forgetDrag(Drag* leaving) = 0;

    // ------------------------------------------------------------
    // Events
    // ------------------------------------------------------------

    virtual void onUp(float dt) override;
    virtual void onDrop(float dt);
    virtual void onPickup(float dt);

    virtual bool shouldBeDown(bool wasDown, bool isHovered, bool mouseDown, bool mousePressed) const override;

    // ------------------------------------------------------------
    // Helpers for subclasses (these touch Drag internals)
    // ------------------------------------------------------------

    // Point piece's back-pointer at this target.
    void adoptDrag(Drag* piece);

    // Clear piece's back-pointer, but only if it still names this target.
    void disownDrag(Drag* piece);

    // True while piece is selected and the mouse is still held, i.e. in the user's hand.
    bool isCarried(const Drag* piece, bool mousePressed) const;

    // False if either this target or piece is locked.
    bool canPickUp(const Drag* piece) const;

    // Move piece to worldXY, keeping its rotation.
    static void placeDrag(Drag* piece, const glm::vec2& worldXY);

    // True if releasing incoming over this target right now would claim it:
    // canAccept, neither this target nor incoming is locked, and incoming's
    // previous owner (if another target) is not locked. Same checks claim() uses.
    bool wouldClaim(const Drag* incoming) const;

    // If piece is in the user's hand and may leave, release it from this target:
    // clear its back-pointer, forgetDrag it, and fire onPickup. Returns true if released.
    bool releaseIfCarried(Drag* piece, bool mousePressed, float dt);

    // The scene's selected drag while the mouse is held, otherwise nullptr.
    Drag* heldDrag(bool mousePressed) const;

    // True if piece is parked in any target.
    static bool isParked(const Drag* piece);

private:
    // Run the claim flow for a selected drag that wouldClaim accepted.
    void claim(Drag* incoming, float dt);
};

} // namespace kerf
