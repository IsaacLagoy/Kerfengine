#include "engine/node/Button.h"
#include "engine/scene/Scene.h"

Button::Button()
    : Model()
{}

Button::Button(const glm::vec3& pose, const glm::vec2& scale, Mesh* mesh, Material* material, Shader* shader, const Collider2D& collider)
    : Model(pose, scale, mesh, material, shader)
{
    this->collider = collider;
}

Button::~Button()
{
    unlinkButton();
}

void Button::update(float dt, const glm::vec2& mousePosition, bool mouseDown, bool mousePressed)
{
    bool wasDown = isDown;
    bool wasHovered = isHovered;

    isHovered = collider.isPointInside(pointToLocalSpace(mousePosition));

    isDown = shouldBeDown(wasDown, isHovered, mouseDown, mousePressed);
    const bool isPressed = isHovered && mousePressed;

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

    if (isPressed) 
    {
        onPressed(dt);
    }

    if (!isPressed)
    {
        onReleased(dt);
    }
}

bool Button::shouldBeDown(bool wasDown, bool isHovered, bool mouseDown, bool mousePressed) const
{
    // mouseDown = press edge this frame; mousePressed = button currently held.
    // Latch only if the press started on this button; keep it until release so
    // dragging onto a button does not count as a click.
    return mousePressed && (wasDown || (mouseDown && isHovered));
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

void Button::insertButton(Button* pos)
{
    unlinkButton();

    this->nextButton = pos;
    this->prevButton = pos->prevButton;
    pos->prevButton->nextButton = this;
    pos->prevButton = this;
}

void Button::enterScene(Scene* scene)
{
    insertButton(scene->getButtonTail());
}

void Button::exitScene(Scene* /*scene*/)
{
    unlinkButton();
}

void Button::unlinkButton()
{
    if (prevButton)
    {
        prevButton->nextButton = nextButton;
    }

    if (nextButton)
    {
        nextButton->prevButton = prevButton;
    }

    nextButton = nullptr;
    prevButton = nullptr;
}