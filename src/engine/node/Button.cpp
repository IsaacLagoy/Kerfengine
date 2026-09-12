#include "engine/node/Button.h"

Button::Button()
    : Model()
{}

Button::Button(const glm::vec3& pose, const glm::vec2& scale, Mesh* mesh, Material* material, Shader* shader, const Collider2D& collider)
    : Model(pose, scale, mesh, material, shader)
{
    this->collider = collider;
}

Button::~Button() {}

void Button::update(float dt, const glm::vec2& mousePosition, bool mouseDown)
{
    bool wasDown = isDown;
    bool wasHovered = isHovered;

    isHovered = collider.isPointInside(pointToLocalSpace(mousePosition));
    isDown = mouseDown && (isHovered || wasDown);

    if (!wasDown && isDown)
    {
        onDown(dt);
    }

    if (wasDown && !isDown)
    {
        onUp(dt);
    }

    if (wasHovered && !isHovered)
    {
        onLeave(dt);
    }

    if (!wasHovered && isHovered)
    {
        onHover(dt);
    }

    if (isDown) 
    {
        onPressed(dt);
    }

    if (!isDown)
    {
        onReleased(dt);
    }
}

void Button::draw(const glm::mat4& viewProjection)
{
    Model::draw(viewProjection);
}

void Button::onDown(float dt)
{
    if (onDownCallback)
    {
        onDownCallback(dt);
    }
}

void Button::onUp(float dt)
{
    if (onUpCallback)
    {
        onUpCallback(dt);
    }
}

void Button::onPressed(float dt)
{
    if (onPressedCallback)
    {
        onPressedCallback(dt);
    }
}

void Button::onReleased(float dt)
{
    if (onReleasedCallback)
    {
        onReleasedCallback(dt);
    }
}

void Button::onHover(float dt)
{
    if (onHoverCallback)
    {
        onHoverCallback(dt);
    }
}

void Button::onLeave(float dt)
{
    if (onLeaveCallback)
    {
        onLeaveCallback(dt);
    }
}