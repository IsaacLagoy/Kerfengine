#include <kerf/engine/node/StickyDrop.h>
#include <kerf/engine/node/Drag.h>
#include <kerf/engine/scene/Scene.h>

#include <glm/glm.hpp>


namespace kerf {

namespace {

constexpr float kMinPullTime = 1.0e-4f;

float sanitizePullTime(float pullTime)
{
    return pullTime < kMinPullTime ? kMinPullTime : pullTime;
}

} // namespace

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
    pullTime(sanitizePullTime(pullTime))
{}

StickyDrop::~StickyDrop()
{
    if (Drag* piece = getDrag())
    {
        piece->setLocked(false);
    }
    lifted = false;
    returning = false;
}

void StickyDrop::update(float dt)
{
    Node::update(dt);

    // nothing to reel in
    Drag* piece = getDrag();
    if (!piece)
    {
        lifted = false;
        returning = false;
        return;
    }

    // dragging is being moved by the user
    Scene* scene = getScene();
    if (scene && scene->getSelectedDrag() == piece)
    {
        lifted = true;
        returning = false;
        piece->setLocked(false);
        return;
    }

    // still sitting in the slot
    // ownership stays on this drop, with no return
    if (!lifted)
    {
        return;
    }

    // needs to be pulled, start returning
    if (!returning)
    {
        returning = true;
        pullElapsed = 0.0f;
        pullFrom = glm::vec2(piece->getWorldPose());
        piece->setLocked(true);
    }

    // interpolate between pullFrom and target
    pullElapsed += dt;
    float t = glm::clamp(pullElapsed / pullTime, 0.0f, 1.0f);
    t = t * t * (3.0f - 2.0f * t);

    const glm::vec2 target(getWorldPose());
    const glm::vec2 pos = glm::mix(pullFrom, target, t);
    piece->setPose(glm::vec3(pos, piece->getWorldPose().z));

    // done returning
    if (pullElapsed >= pullTime)
    {
        lifted = false;
        returning = false;
        piece->setLocked(false);
        const glm::vec3 dropPose = getWorldPose();
        piece->setPose(glm::vec3(dropPose.x, dropPose.y, piece->getWorldPose().z));
    }
}

void StickyDrop::updateButton(float dt, const glm::vec2& mousePosition, bool mouseDown, bool mousePressed)
{
    Button::updateButton(dt, mousePosition, mouseDown, mousePressed);

    // locked while returning: keep ownership and let update() ease the pose.
    if (returning)
    {
        return;
    }

    // no drag
    Drag* piece = getDrag();
    if (!piece)
    {
        return;
    }

    // still owned while in hand, including the release frame before another drop can claim.
    Scene* scene = getScene();
    if (scene && scene->getSelectedDrag() == piece)
    {
        return;
    }

    // recenter drag to here
    const glm::vec3 dropPose = getWorldPose();
    piece->setPose(glm::vec3(dropPose.x, dropPose.y, piece->getWorldPose().z));
}

float StickyDrop::getPullTime() const
{
    return pullTime;
}

void StickyDrop::setPullTime(float pullTime)
{
    // clamp into bounds
    this->pullTime = sanitizePullTime(pullTime);
}

void StickyDrop::setDrag(Drag* next)
{
    // unlock previous drag
    Drag* previous = getDrag();
    if (previous && previous != next)
    {
        previous->setLocked(false);
    }

    // reset state
    lifted = false;
    returning = false;
    assignParkedDrop(next);

    // unlock new drag
    if (next)
    {
        next->setLocked(false);
    }
}

void StickyDrop::onDrop(float dt)
{
    // reset state
    lifted = false;
    returning = false;

    // unlock drag
    if (Drag* piece = getDrag())
    {
        piece->setLocked(false);
    }
    Drop::onDrop(dt);
}

void StickyDrop::onPickup(float dt)
{
    // reset state
    lifted = false;
    returning = false;
    Drop::onPickup(dt);
}

bool StickyDrop::shouldBeDown(bool wasDown, bool isHovered, bool mouseDown, bool mousePressed) const
{
    if (returning)
    {
        return false;
    }
    return Drop::shouldBeDown(wasDown, isHovered, mouseDown, mousePressed);
}

void StickyDrop::setLifted(bool lifted)
{
    this->lifted = lifted;
}

} // namespace kerf
