#include <kerf/engine/node/drop/StickyDrop.h>
#include <kerf/engine/node/Drag.h>
#include <kerf/engine/scene/Scene.h>

#include <glm/glm.hpp>


namespace kerf {

StickyDrop::StickyDrop(
    const glm::vec3& pose,
    const glm::vec2& scale,
    Mesh* mesh,
    Material* material,
    Shader* shader,
    const Collider2D& collider,
    float pullTime
) :
    Drop(pose, scale, mesh, material, shader, collider),
    pullBack(pullTime)
{}

StickyDrop::~StickyDrop()
{
    if (Drag* piece = getDrag())
    {
        piece->setLocked(false);
    }
    pullBack.seat();
}

void StickyDrop::update(float dt)
{
    Node::update(dt);

    // nothing to reel in
    Drag* piece = getDrag();
    if (!piece)
    {
        pullBack.seat();
        return;
    }

    // drag is being moved by the user
    Scene* scene = getScene();
    if (scene && scene->getSelectedDrag() == piece)
    {
        pullBack.lift();
        piece->setLocked(false);
        return;
    }

    // still sitting in the slot
    // ownership stays on this drop, with no return
    if (pullBack.isSeated())
    {
        return;
    }

    // needs to be pulled, start returning
    if (pullBack.beginReturn(glm::vec2(piece->getWorldPose())))
    {
        piece->setLocked(true);
    }

    // ease toward the slot; advance() lands exactly on it when done
    placeDrag(piece, pullBack.advance(dt, glm::vec2(getWorldPose())));

    // done returning
    if (pullBack.isSeated())
    {
        piece->setLocked(false);
    }
}

void StickyDrop::updateButton(float dt, const glm::vec2& mousePosition, bool mouseDown, bool mousePressed)
{
    Button::updateButton(dt, mousePosition, mouseDown, mousePressed);

    Drag* piece = getDrag();
    if (!piece)
    {
        return;
    }

    Scene* scene = getScene();
    if (scene && scene->getSelectedDrag() == piece)
    {
        piece->setLocked(false);

        // selectedDrag stays set through the release frame so drops can claim.
        // Only treat the piece as in-hand while the mouse is still down.
        if (mousePressed)
        {
            pullBack.lift();
            return;
        }

        // released back onto this drop
        if (getIsHovered())
        {
            pullBack.seat();
            placeDrag(piece, glm::vec2(getWorldPose()));
            return;
        }

        // released elsewhere, update() starts the return
        pullBack.lift();
        return;
    }

    // keep ownership and let update() ease the pose while returning
    if (pullBack.isReturning())
    {
        return;
    }

    // recenter drag to here
    placeDrag(piece, glm::vec2(getWorldPose()));
}

float StickyDrop::getPullTime() const
{
    return pullBack.getDuration();
}

void StickyDrop::setPullTime(float pullTime)
{
    pullBack.setDuration(pullTime);
}

void StickyDrop::setDrag(Drag* next)
{
    if (isLocked())
    {
        return;
    }

    // unlock previous drag
    Drag* previous = getDrag();
    if (previous && previous != next)
    {
        previous->setLocked(false);
    }

    pullBack.seat();
    assignParkedDrop(next);
}

void StickyDrop::onDrop(float dt)
{
    pullBack.seat();
    Drop::onDrop(dt);
}

void StickyDrop::onPickup(float dt)
{
    pullBack.seat();
    Drop::onPickup(dt);
}

bool StickyDrop::shouldBeDown(bool wasDown, bool isHovered, bool mouseDown, bool mousePressed) const
{
    if (pullBack.isReturning())
    {
        return false;
    }
    return Drop::shouldBeDown(wasDown, isHovered, mouseDown, mousePressed);
}

void StickyDrop::setLifted(bool lifted)
{
    if (lifted)
    {
        // a return already in progress keeps going
        if (pullBack.isSeated())
        {
            pullBack.lift();
        }
        return;
    }

    // cancelling a return: release the lock it took, updateButton re-centers the drag
    if (pullBack.isReturning())
    {
        if (Drag* piece = getDrag())
        {
            piece->setLocked(false);
        }
    }
    pullBack.seat();
}

} // namespace kerf
