#include "engine/node/Drag.h"

Drag::Drag(const glm::vec3& pose, const glm::vec2& scale, Mesh* mesh, Material* material, Shader* shader, const Collider2D& collider)
    : Button(pose, scale, mesh, material, shader, collider)
{
    relativePosition = glm::vec2(0.0f);
    velocity = glm::vec2(0.0f);
    isDragging = false;
}

Drag::~Drag() {}

void Drag::update(float dt, const glm::vec2& mousePosition, bool mouseDown)
{
    Button::update(dt, mousePosition, mouseDown);

    if (!isDragging)
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

void Drag::onDown(float dt)
{
    isDragging = true;

    // preserve button onDown callback
    Button::onDown(dt);
}

void Drag::onUp(float dt)
{
    isDragging = false;

    // preserve button onUp callback
    Button::onUp(dt);
}