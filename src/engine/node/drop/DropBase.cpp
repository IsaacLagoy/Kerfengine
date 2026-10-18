#include <kerf/engine/node/drop/DropBase.h>
#include <kerf/engine/node/Drag.h>
#include <kerf/engine/scene/Scene.h>


namespace kerf {

DropBase::DropBase(const glm::vec3& pose, const glm::vec2& scale, Mesh* mesh, Material* material, Shader* shader, const Collider2D& collider)
    : Button(pose, scale, mesh, material, shader, collider)
{}

bool DropBase::isLocked() const
{
    return locked;
}

void DropBase::setLocked(bool locked)
{
    this->locked = locked;
}

void DropBase::setOnDropCallback(const std::function<void(float)>& callback)
{
    onDropCallback = callback;
}

void DropBase::setOnPickupCallback(const std::function<void(float)>& callback)
{
    onPickupCallback = callback;
}

bool DropBase::shouldBeDown(bool wasDown, bool isHovered, bool mouseDown, bool mousePressed) const
{
    (void) wasDown;
    
    // Do not latch off-hover. A drop is down while the cursor is over it and
    // either the press started here or a drag is being held (so onUp can claim).
    if (!mousePressed || !isHovered)
    {
        return false;
    }

    return mouseDown || getScene()->getSelectedDrag() != nullptr;
}

void DropBase::onUp(float dt)
{
    if (getIsHovered())
    {
        Drag* selected = getScene()->getSelectedDrag();
        if (selected != nullptr && wouldClaim(selected))
        {
            claim(selected, dt);
        }
    }

    Button::onUp(dt);
}

bool DropBase::wouldClaim(const Drag* incoming) const
{
    if (!canAccept(incoming) || locked || incoming->locked)
    {
        return false;
    }

    const DropBase* previous = incoming->parkedDrop;
    return !(previous && previous != this && previous->locked);
}

void DropBase::claim(Drag* incoming, float dt)
{
    // take it from wherever it is parked now
    DropBase* previous = incoming->parkedDrop;
    if (previous && previous != this)
    {
        previous->forgetDrag(incoming);
        previous->onPickup(dt);
    }

    incoming->parkedDrop = this;
    accept(incoming);
    onDrop(dt);
}

void DropBase::onDrop(float dt)
{
    if (onDropCallback)
    {
        onDropCallback(dt);
    }
}

void DropBase::onPickup(float dt)
{
    if (onPickupCallback)
    {
        onPickupCallback(dt);
    }
}

void DropBase::adoptDrag(Drag* piece)
{
    piece->parkedDrop = this;
}

void DropBase::disownDrag(Drag* piece)
{
    if (piece->parkedDrop == this)
    {
        piece->parkedDrop = nullptr;
    }
}

bool DropBase::isCarried(const Drag* piece, bool mousePressed) const
{
    return piece->isDragging() && mousePressed;
}

bool DropBase::canPickUp(const Drag* piece) const
{
    return !locked && !piece->locked;
}

bool DropBase::releaseIfCarried(Drag* piece, bool mousePressed, float dt)
{
    if (!isCarried(piece, mousePressed) || !canPickUp(piece))
    {
        return false;
    }

    disownDrag(piece);
    forgetDrag(piece);
    onPickup(dt);
    return true;
}

Drag* DropBase::heldDrag(bool mousePressed) const
{
    Scene* scene = getScene();
    if (!mousePressed || !scene)
    {
        return nullptr;
    }
    return scene->getSelectedDrag();
}

bool DropBase::isParked(const Drag* piece)
{
    return piece->parkedDrop != nullptr;
}

void DropBase::placeDrag(Drag* piece, const glm::vec2& worldXY)
{
    piece->setPose(glm::vec3(worldXY, piece->getWorldPose().z));
}

} // namespace kerf
