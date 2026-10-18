#pragma once

#include <kerf/engine/node/drop/DropBase.h>

#include <vector>


namespace kerf {

// forward declarations
class Drag;

/**
 * @brief A drop target that holds many drags in a row, like a hand of cards.
 *
 * Resting cards are laid out CSS "space-around" along the accordion's axis: n cards
 * over its length each get an equal share and sit centered in it. Once a share is
 * narrower than a card, the cards overlap.
 *
 * A held drag over the accordion that it would accept is the "pending" card. It gets
 * its own slot in the order (the gap) at the share under the cursor, and releasing it
 * commits exactly that order. This is the only path: a card picked up from this
 * accordion, from another target, or from nowhere is handled the same way, which
 * also makes reordering fall out for free. A held card the row still keeps (locked
 * row, or a sticky row) is pending at its home slot while the cursor is elsewhere.
 *
 * The axis is the accordion's local x (index 0 at local -x), so it follows the
 * accordion's position and rotation. For a vertical column with index 0 at the top,
 * rotate it by -pi/2. Length is the box collider's width times world scale x.
 */
class DropAccordion : public DropBase {
private:
    struct Item {
        Drag* drag;
        glm::vec2 pos; // (along, across) the axis, relative to the center, world units
    };

    std::vector<Item> items;
    float followSpeed;
    float itemLayer = 1.0f;
    int capacity = -1;

    Drag* pending = nullptr;
    int pendingIndex = -1;
    glm::vec2 lastMouse = glm::vec2(0.0f);

public:
    DropAccordion(
        const glm::vec3& pose,
        const glm::vec2& scale,
        Mesh* mesh,
        Material* material,
        Shader* shader,
        const Collider2D& collider,
        float followSpeed = 14.0f
    );
    ~DropAccordion() override;

    virtual void updateButton(float dt, const glm::vec2& mousePosition, bool mouseDown, bool mousePressed) override;

    // Cards in order, with the pending card (if any) inserted where it would land.
    std::vector<Drag*> getOrder() const;

    // getters
    Drag* getPending() const;     // nullptr when no card is about to land
    int getPendingIndex() const;  // -1 when none
    int getCount() const;         // resting cards only, excludes pending
    Drag* getDrag(int index) const;
    int getCapacity() const;
    float getFollowSpeed() const;
    float getItemLayer() const;
    float getLength() const;      // world length along the axis

    // setters
    void setCapacity(int capacity);          // -1 = unlimited
    void setFollowSpeed(float followSpeed);  // <= 0 snaps instantly
    void setItemLayer(float layer);          // card i draws at layer + i * 0.01

    // Setup: park drag at index (-1 = end). Refuses if locked, full, or the drag is
    // already parked somewhere. Returns true if inserted.
    bool insertDrag(Drag* drag, int index = -1);

protected:
    bool canAccept(const Drag* incoming) const override;
    void accept(Drag* incoming) override;
    void forgetDrag(Drag* leaving) override;

    // ------------------------------------------------------------
    // Hooks for subclasses (StickyDropAccordion)
    // ------------------------------------------------------------

    // False (default): a card picked up leaves the row at once. True: the row keeps it
    // while carried; it stays pending at its home slot until another target claims it.
    virtual bool keepsCarried() const;

    // Step a resting card from pos toward its slot target (both in frame space).
    // Default: smooth follow at followSpeed.
    virtual glm::vec2 moveItem(Drag* drag, const glm::vec2& pos, const glm::vec2& target, float dt);

    // world <-> (along, across) the axis, relative to the center
    glm::vec2 toFrame(const glm::vec2& world) const;
    glm::vec2 toWorld(const glm::vec2& frame) const;

private:
    glm::vec2 axis() const;
    int indexOf(const Drag* drag) const;
    // insertion index for a card at world, among the other cards (excluding moving)
    int slotUnder(const glm::vec2& world, const Drag* moving) const;
    void layout(float dt, bool mousePressed);
};

} // namespace kerf
