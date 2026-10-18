#include <kerf/engine/node/drop/Drop.h>
#include <kerf/engine/node/Drag.h>


namespace kerf {

Drop::Drop(const glm::vec3& pose, const glm::vec2& scale, Mesh* mesh, Material* material, Shader* shader, const Collider2D& collider)
    : DropBase(pose, scale, mesh, material, shader, collider)
{}

Drop::~Drop()
{
    if (!drag)
    {
        return;
    }

    disownDrag(drag);
    drag = nullptr;
}

void Drop::forgetDrag(Drag* leaving)
{
    if (drag == leaving)
    {
        drag = nullptr;
    }
}

bool Drop::canAccept(const Drag* incoming) const
{
    (void) incoming;
    return drag == nullptr;
}

void Drop::accept(Drag* incoming)
{
    drag = incoming;
    placeDrag(incoming, glm::vec2(getWorldPose()));
}

void Drop::updateButton(float dt, const glm::vec2& mousePosition, bool mouseDown, bool mousePressed)
{
    Button::updateButton(dt, mousePosition, mouseDown, mousePressed);

    if (!drag)
    {
        return;
    }

    // Piece is in the user's hand: release it if allowed, never re-center it.
    if (isCarried(drag, mousePressed))
    {
        releaseIfCarried(drag, mousePressed, dt);
        return;
    }

    placeDrag(drag, glm::vec2(getWorldPose()));
}

Drag* Drop::getDrag() const
{
    return drag;
}

void Drop::setDrag(Drag* drag)
{
    if (isLocked())
    {
        return;
    }
    this->drag = drag;
}

// park next here and keep Drag::parkedDrop in sync
// clears the previous drag's back-pointer when it still names this drop.
void Drop::assignParkedDrop(Drag* next)
{
    if (isLocked())
    {
        return;
    }

    if (drag && drag != next)
    {
        disownDrag(drag);
    }
    drag = next;
    if (next)
    {
        adoptDrag(next);
    }
}

} // namespace kerf
