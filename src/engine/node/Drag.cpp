#include <kerf/engine/node/Drag.h>
#include <kerf/engine/scene/Scene.h>


namespace kerf {

Drag::Drag(const glm::vec3& pose, const glm::vec2& scale, Mesh* mesh, Material* material, Shader* shader, const Collider2D& collider)
    : Button(pose, scale, mesh, material, shader, collider)
{
    relativePosition = glm::vec2(0.0f);
    velocity = glm::vec2(0.0f);
}

Drag::~Drag() {}

void Drag::updateButton(float dt, const glm::vec2& mousePosition, bool mouseDown, bool mousePressed)
{
    Button::updateButton(dt, mousePosition, mouseDown, mousePressed);

    if (locked)
    {
        wasDragging = false;
        getScene()->setSelectedDrag(nullptr);
        return;
    }

    // Follow the mouse only while this drag is selected and the button is held.
    // selectedDrag stays set until Scene::update ends so drops can claim on release.
    if (!isDragging() || !mousePressed)
    {
        wasDragging = false;
        velocity = glm::vec2(0.0f);
        return;
    }

    const glm::vec2 pose2 = glm::vec2(getPose());
    if (!wasDragging)
    {
        // grab offset in world space, first frame has no previous mouse sample
        relativePosition = mousePosition - pose2;
    }

    const glm::vec2 target = mousePosition - relativePosition;
    velocity = target - pose2;
    setPose(glm::vec3(target, getPose().z));
    wasDragging = true;
}

void Drag::setOnDropCallback(const std::function<void(float)>& callback)
{
    onDropCallback = callback;
}

void Drag::setOnPickupCallback(const std::function<void(float)>& callback)
{
    onPickupCallback = callback;
}

void Drag::onDown(float dt)
{
    // reserve self as selected drag
    if (getScene()->getSelectedDrag() == nullptr)
    {
        getScene()->setSelectedDrag(this);
        onPickup(dt);
    }

    // preserve button onDown callback
    Button::onDown(dt);
}

void Drag::onUp(float dt)
{
    // Leave selectedDrag set for the rest of this frame so a Drop::onUp can claim it.
    if (isDragging())
    {
        onDrop(dt);
    }
    Button::onUp(dt);
}

void Drag::onDrop(float dt)
{
    if (onDropCallback)
    {
        onDropCallback(dt);
    }
}
void Drag::onPickup(float dt)
{
    if (onPickupCallback)
    {
        onPickupCallback(dt);
    }
}

bool Drag::isDragging() const
{
    return getScene()->getSelectedDrag() == this;
}

} // namespace kerf