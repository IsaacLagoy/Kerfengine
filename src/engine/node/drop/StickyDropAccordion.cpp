#include <kerf/engine/node/drop/StickyDropAccordion.h>
#include <kerf/engine/node/Drag.h>
#include <kerf/engine/scene/Scene.h>


namespace kerf {

StickyDropAccordion::StickyDropAccordion(
    const glm::vec3& pose,
    const glm::vec2& scale,
    Mesh* mesh,
    Material* material,
    Shader* shader,
    const Collider2D& collider,
    float pullTime,
    float followSpeed
) :
    DropAccordion(pose, scale, mesh, material, shader, collider, followSpeed),
    pullTime(PullBack(pullTime).getDuration())
{}

StickyDropAccordion::~StickyDropAccordion()
{
    // release the locks taken by returns in progress
    for (int i = 0; i < getCount(); ++i)
    {
        if (isReturning(getDrag(i)))
        {
            getDrag(i)->setLocked(false);
        }
    }
    pulls.clear();
}

void StickyDropAccordion::update(float dt)
{
    DropAccordion::update(dt);

    // Runs before buttons, so a card released over another target has already been
    // claimed (and forgotten here) by the time its return would start.
    Scene* scene = getScene();
    Drag* selected = scene ? scene->getSelectedDrag() : nullptr;

    for (int i = 0; i < getCount(); ++i)
    {
        Drag* drag = getDrag(i);
        PullBack& pull = pullFor(drag);

        // in the user's hand
        if (drag == selected)
        {
            pull.lift();
            continue;
        }

        // released somewhere that did not claim it: pull it back
        if (pull.beginReturn(toFrame(glm::vec2(drag->getWorldPose()))))
        {
            drag->setLocked(true);
        }
    }
}

bool StickyDropAccordion::keepsCarried() const
{
    return true;
}

glm::vec2 StickyDropAccordion::moveItem(Drag* drag, const glm::vec2& pos, const glm::vec2& target, float dt)
{
    PullBack& pull = pullFor(drag);
    if (!pull.isReturning())
    {
        return DropAccordion::moveItem(drag, pos, target, dt);
    }

    // advance() tracks the live slot, which may move while the card is on its way
    const glm::vec2 next = pull.advance(dt, target);
    if (pull.isSeated())
    {
        drag->setLocked(false);
    }
    return next;
}

void StickyDropAccordion::forgetDrag(Drag* leaving)
{
    if (isReturning(leaving))
    {
        leaving->setLocked(false);
    }
    pulls.erase(leaving);
    DropAccordion::forgetDrag(leaving);
}

PullBack& StickyDropAccordion::pullFor(const Drag* drag)
{
    return pulls.try_emplace(drag, pullTime).first->second;
}

float StickyDropAccordion::getPullTime() const
{
    return pullTime;
}

bool StickyDropAccordion::isReturning(const Drag* drag) const
{
    const auto it = pulls.find(drag);
    return it != pulls.end() && it->second.isReturning();
}

void StickyDropAccordion::setPullTime(float pullTime)
{
    this->pullTime = PullBack(pullTime).getDuration();
    for (auto& [drag, pull] : pulls)
    {
        pull.setDuration(this->pullTime);
    }
}

void StickyDropAccordion::setLifted(Drag* drag, bool lifted)
{
    bool here = false;
    for (int i = 0; i < getCount() && !here; ++i)
    {
        here = getDrag(i) == drag;
    }
    if (!here)
    {
        return;
    }

    PullBack& pull = pullFor(drag);
    if (lifted)
    {
        // a return already in progress keeps going
        if (pull.isSeated())
        {
            pull.lift();
        }
        return;
    }

    if (pull.isReturning())
    {
        drag->setLocked(false);
    }
    pull.seat();
}

} // namespace kerf
