#include <kerf/engine/node/drop/DropAccordion.h>
#include <kerf/engine/node/drop/AccordionLayout.h>
#include <kerf/engine/node/Drag.h>

#include <algorithm>
#include <cmath>
#include <glm/glm.hpp>


namespace kerf {

namespace {

constexpr float kLayerStep = 0.01f;

} // namespace

DropAccordion::DropAccordion(
    const glm::vec3& pose,
    const glm::vec2& scale,
    Mesh* mesh,
    Material* material,
    Shader* shader,
    const Collider2D& collider,
    float followSpeed
) :
    DropBase(pose, scale, mesh, material, shader, collider),
    followSpeed(followSpeed)
{}

DropAccordion::~DropAccordion()
{
    for (const Item& item : items)
    {
        disownDrag(item.drag);
    }
    items.clear();
    pending = nullptr;
}

void DropAccordion::updateButton(float dt, const glm::vec2& mousePosition, bool mouseDown, bool mousePressed)
{
    // may claim on release, which calls accept()
    lastMouse = mousePosition;
    Button::updateButton(dt, mousePosition, mouseDown, mousePressed);

    // cards in the user's hand leave the row, unless this row keeps them
    // (copy first: releasing erases from items)
    if (!keepsCarried())
    {
        std::vector<Drag*> resting;
        resting.reserve(items.size());
        for (const Item& item : items)
        {
            resting.push_back(item.drag);
        }
        for (Drag* drag : resting)
        {
            releaseIfCarried(drag, mousePressed, dt);
        }
    }

    // The held card is about to land here if releasing it would claim it (slot under
    // the cursor), or if this row still holds it (its home slot: it will come back).
    pending = nullptr;
    pendingIndex = -1;
    if (Drag* held = heldDrag(mousePressed))
    {
        const int home = indexOf(held);
        if (getIsHovered() && wouldClaim(held))
        {
            pending = held;
            pendingIndex = slotUnder(mousePosition, held);
        }
        else if (home >= 0)
        {
            pending = held;
            pendingIndex = home;
        }
    }

    layout(dt, mousePressed);
}

void DropAccordion::layout(float dt, bool mousePressed)
{
    const std::vector<Drag*> order = getOrder();
    const int count = static_cast<int>(order.size());
    const float length = getLength();

    for (Item& item : items)
    {
        // in the user's hand: it follows the mouse, its slot is the gap
        if (isCarried(item.drag, mousePressed))
        {
            item.pos = toFrame(glm::vec2(item.drag->getWorldPose()));
            continue;
        }

        const int slot = static_cast<int>(std::find(order.begin(), order.end(), item.drag) - order.begin());
        const glm::vec2 target(accordion::slotOffset(slot, count, length), 0.0f);

        item.pos = moveItem(item.drag, item.pos, target, dt);
        placeDrag(item.drag, toWorld(item.pos));
        item.drag->setLayer(itemLayer + static_cast<float>(slot) * kLayerStep);
    }
}

glm::vec2 DropAccordion::moveItem(Drag* /*drag*/, const glm::vec2& pos, const glm::vec2& target, float dt)
{
    const float blend = followSpeed > 0.0f ? 1.0f - std::exp(-followSpeed * dt) : 1.0f;
    return pos + (target - pos) * blend;
}

bool DropAccordion::keepsCarried() const
{
    return false;
}

std::vector<Drag*> DropAccordion::getOrder() const
{
    std::vector<Drag*> order;
    order.reserve(items.size() + 1);
    for (const Item& item : items)
    {
        if (item.drag != pending)
        {
            order.push_back(item.drag);
        }
    }
    if (pending)
    {
        order.insert(order.begin() + std::clamp(pendingIndex, 0, static_cast<int>(order.size())), pending);
    }
    return order;
}

int DropAccordion::indexOf(const Drag* drag) const
{
    for (int k = 0; k < static_cast<int>(items.size()); ++k)
    {
        if (items[k].drag == drag)
        {
            return k;
        }
    }
    return -1;
}

bool DropAccordion::canAccept(const Drag* incoming) const
{
    // a card already here is a move, not an extra card
    return indexOf(incoming) >= 0 || capacity < 0 || static_cast<int>(items.size()) < capacity;
}

void DropAccordion::accept(Drag* incoming)
{
    // commit what the preview showed; fall back to the cursor if it was never pending
    int index = (incoming == pending) ? pendingIndex : -1;

    // still listed here (this row kept it while carried): move it instead
    forgetDrag(incoming);
    if (index < 0)
    {
        index = slotUnder(lastMouse, nullptr);
    }

    // glide in from wherever it was released
    const glm::vec2 from = toFrame(glm::vec2(incoming->getWorldPose()));
    items.insert(items.begin() + std::clamp(index, 0, static_cast<int>(items.size())), Item{incoming, from});

    pending = nullptr;
    pendingIndex = -1;
}

void DropAccordion::forgetDrag(Drag* leaving)
{
    items.erase(
        std::remove_if(items.begin(), items.end(), [leaving](const Item& item) { return item.drag == leaving; }),
        items.end()
    );
    if (pending == leaving)
    {
        pending = nullptr;
        pendingIndex = -1;
    }
}

bool DropAccordion::insertDrag(Drag* drag, int index)
{
    if (!drag || isLocked() || !canAccept(drag) || isParked(drag))
    {
        return false;
    }

    const int count = static_cast<int>(items.size());
    if (index < 0 || index > count)
    {
        index = count;
    }

    adoptDrag(drag);
    const glm::vec2 slot(accordion::slotOffset(index, count + 1, getLength()), 0.0f);
    items.insert(items.begin() + index, Item{drag, slot});
    placeDrag(drag, toWorld(slot));
    return true;
}

// ------------------------------------------------------------
// Geometry
// ------------------------------------------------------------

float DropAccordion::getLength() const
{
    const Collider2D& collider = getCollider();
    const float scaleX = getWorldScale().x;

    if (collider.type == ColliderType::BOX)
    {
        return collider.boxShape.width * scaleX;
    }
    return 2.0f * scaleX;
}

glm::vec2 DropAccordion::axis() const
{
    // local +x
    const float r = getWorldPose().z;
    return glm::vec2(std::cos(r), std::sin(r));
}

glm::vec2 DropAccordion::toFrame(const glm::vec2& world) const
{
    const glm::vec2 a = axis();
    const glm::vec2 perp(-a.y, a.x);
    const glm::vec2 d = world - glm::vec2(getWorldPose());
    return glm::vec2(glm::dot(d, a), glm::dot(d, perp));
}

glm::vec2 DropAccordion::toWorld(const glm::vec2& frame) const
{
    const glm::vec2 a = axis();
    const glm::vec2 perp(-a.y, a.x);
    return glm::vec2(getWorldPose()) + a * frame.x + perp * frame.y;
}

int DropAccordion::slotUnder(const glm::vec2& world, const Drag* moving) const
{
    // slots for the other cards, plus one for the incoming card
    const int others = static_cast<int>(items.size()) - (indexOf(moving) >= 0 ? 1 : 0);
    return accordion::slotAt(toFrame(world).x, others + 1, getLength());
}

// ------------------------------------------------------------
// Getters / setters
// ------------------------------------------------------------

Drag* DropAccordion::getPending() const 
{ 
    return pending; 
}

int DropAccordion::getPendingIndex() const 
{ 
    return pendingIndex; 
}

int DropAccordion::getCount() const 
{ 
    return static_cast<int>(items.size()); 
}

Drag* DropAccordion::getDrag(int index) const
{
    if (index < 0 || index >= static_cast<int>(items.size()))
    {
        return nullptr;
    }
    return items[index].drag;
}

int DropAccordion::getCapacity() const 
{ 
    return capacity; 
}

float DropAccordion::getFollowSpeed() const 
{ 
    return followSpeed; 
}

float DropAccordion::getItemLayer() const 
{ 
    return itemLayer; 
}

void DropAccordion::setCapacity(int capacity) 
{ 
    this->capacity = capacity < 0 ? -1 : capacity; 
}

void DropAccordion::setFollowSpeed(float followSpeed) 
{ 
    this->followSpeed = followSpeed; 
}

void DropAccordion::setItemLayer(float layer) 
{ 
    itemLayer = layer; 
}

} // namespace kerf
