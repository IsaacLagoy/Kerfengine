#include <kerf/engine/node/Drop.h>
#include <kerf/engine/node/Drag.h>
#include <kerf/engine/scene/Scene.h>


namespace kerf {

Drop::Drop(const glm::vec3& pose, const glm::vec2& scale, Mesh* mesh, Material* material, Shader* shader, const Collider2D& collider)
    : Button(pose, scale, mesh, material, shader, collider)
{}

Drop::~Drop()
{
    if (!drag)
    {
        return;
    }

    if (drag->parkedDrop == this)
    {
        drag->parkedDrop = nullptr;
    }
    drag = nullptr;
}

void Drop::forgetDrag(Drag* leaving)
{
    if (drag == leaving)
    {
        drag = nullptr;
    }
}

void Drop::updateButton(float dt, const glm::vec2& mousePosition, bool mouseDown, bool mousePressed)
{
    Button::updateButton(dt, mousePosition, mouseDown, mousePressed);

    if (!drag)
    {
        return;
    }

    // Piece was picked up from this slot.
    if (drag->isDragging() && mousePressed)
    {
        if (drag->parkedDrop == this)
        {
            drag->parkedDrop = nullptr;
        }
        drag = nullptr;
        onPickup(dt);
        return;
    }

    const glm::vec3 dropPose = getWorldPose();
    drag->setPose(glm::vec3(dropPose.x, dropPose.y, drag->getWorldPose().z));
}

Drag* Drop::getDrag() const
{
    return drag;
}

void Drop::setDrag(Drag* drag)
{
    this->drag = drag;
}

// park next here and keep Drag::parkedDrop in sync
// clears the previous drag's back-pointer when it still names this drop.
void Drop::assignParkedDrop(Drag* next)
{

    if (drag && drag != next && drag->parkedDrop == this)
    {
        drag->parkedDrop = nullptr;
    }
    drag = next;
    if (next)
    {
        next->parkedDrop = this;
    }
}

void Drop::setOnDropCallback(const std::function<void(float)>& callback)
{
    onDropCallback = callback;
}

void Drop::setOnPickupCallback(const std::function<void(float)>& callback)
{
    onPickupCallback = callback;
}

bool Drop::shouldBeDown(bool /*wasDown*/, bool isHovered, bool mouseDown, bool mousePressed) const
{
    // Do not latch off-hover. A drop is down while the cursor is over it and
    // either the press started here or a drag is being held (so onUp can claim).
    if (!mousePressed || !isHovered)
    {
        return false;
    }

    return mouseDown || getScene()->getSelectedDrag() != nullptr;
}

void Drop::onDown(float dt)
{
    Button::onDown(dt);
}

void Drop::onDrop(float dt)
{
    if (onDropCallback)
    {
        onDropCallback(dt);
    }
}

void Drop::onPickup(float dt)
{
    if (onPickupCallback)
    {
        onPickupCallback(dt);
    }
}

void Drop::onUp(float dt)
{
    if (drag == nullptr && getIsHovered())
    {
        Drag* selected = getScene()->getSelectedDrag();
        if (selected != nullptr)
        {
            if (selected->locked)
            {
                Button::onUp(dt);
                return;
            }
            if (selected->parkedDrop && selected->parkedDrop != this)
            {
                selected->parkedDrop->drag = nullptr;
                selected->parkedDrop->onPickup(dt);
            }
            drag = selected;
            selected->parkedDrop = this;
            onDrop(dt);
        }
    }

    Button::onUp(dt);
}

} // namespace kerf