#include "render/camera/Camera.h"
#include <glm/gtc/matrix_transform.hpp>


Camera::Camera(int viewportWidth, int viewportHeight, float zoom) : 
      zoom(zoom)
{
    resize(viewportWidth, viewportHeight);
    updateProjection();
    updateView();
}

Camera::~Camera() {}

void Camera::resize(int viewportWidth, int viewportHeight)
{
    this->viewportWidth = static_cast<float>(viewportWidth);
    this->viewportHeight = static_cast<float>(viewportHeight);
    setAspect(this->viewportWidth / this->viewportHeight);
}

void Camera::setAspect(float aspect)
{
    this->aspect = aspect;
    updateProjection();
}

void Camera::setZoom(float zoom)
{
    this->zoom = zoom;
    updateProjection();
}

void Camera::setPosition(const glm::vec2& position)
{
    this->position = position;
    updateView();
}

glm::mat4 Camera::getProjection() const
{
    return proj;
}

glm::mat4 Camera::getView() const
{
    return view;
}

glm::vec2 Camera::getPosition() const
{
    return position;
}

glm::vec2 Camera::screenToWorld(const glm::vec2& screen) const
{
    // GLFW / window pixels: origin top-left, Y down
    const glm::vec2 ndc(
        (screen.x / viewportWidth) * 2.0f - 1.0f,
        1.0f - (screen.y / viewportHeight) * 2.0f
    );

    const glm::vec4 world = glm::inverse(proj * view) * glm::vec4(ndc, 0.0f, 1.0f);
    return glm::vec2(world);
}

glm::vec2 Camera::worldToScreen(const glm::vec2& world) const
{
    const glm::vec4 clip = (proj * view) * glm::vec4(world, 0.0f, 1.0f);
    const glm::vec2 ndc = glm::vec2(clip) / clip.w;

    return glm::vec2(
        (ndc.x + 1.0f) * 0.5f * viewportWidth,
        (1.0f - ndc.y) * 0.5f * viewportHeight
    );
}

void Camera::updateProjection()
{
    const float halfH = 1.0f / zoom;
    const float halfW = halfH * aspect;
    proj = glm::ortho(-halfW, halfW, -halfH, halfH, -1.0f, 1.0f);
}

void Camera::updateView()
{
    view = glm::translate(glm::mat4(1.0f), glm::vec3(-position, 0.0f));
}
