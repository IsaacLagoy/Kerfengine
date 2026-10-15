#include <kerf/engine/render/camera/Camera.h>
#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>

namespace kerf {
    
Camera::Camera(int viewportWidth, int viewportHeight, float zoom) : 
      zoom(zoom)
{
    resize(viewportWidth, viewportHeight);
    setScreenRect(0.0f, 0.0f, static_cast<float>(viewportWidth), static_cast<float>(viewportHeight));
    updateProjection();
    updateView();
}

Camera::~Camera() {}

void Camera::resize(int viewportWidth, int viewportHeight)
{
    // only resize if the viewport has changed
    if (this->viewportWidth == viewportWidth && this->viewportHeight == viewportHeight) return;

    this->viewportWidth = static_cast<float>(viewportWidth);
    this->viewportHeight = static_cast<float>(viewportHeight);
    setAspect(this->viewportWidth / this->viewportHeight);
}

void Camera::setScreenRect(float x, float y, float width, float height)
{
    screenRect = glm::vec4(x, y, std::max(width, 1.0f), std::max(height, 1.0f));
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

void Camera::setClipRange(float near, float far)
{
    this->near = near;
    this->far = far;
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

float Camera::getNear() const
{
    return near;
}

float Camera::getFar() const
{
    return far;
}

glm::vec2 Camera::screenToWorld(const glm::vec2& screen) const
{
    // GLFW / window pixels: origin top-left, Y down
    const glm::vec2 ndc(
        ((screen.x - screenRect.x) / screenRect.z) * 2.0f - 1.0f,
        1.0f - ((screen.y - screenRect.y) / screenRect.w) * 2.0f
    );

    const glm::vec4 world = glm::inverse(proj * view) * glm::vec4(ndc, 0.0f, 1.0f);
    return glm::vec2(world);
}

std::map<std::string, glm::vec2> Camera::getWindowCorners() const
{
    const float left = screenRect.x;
    const float top = screenRect.y;
    const float right = screenRect.x + screenRect.z;
    const float bottom = screenRect.y + screenRect.w;
    return {
        { "topLeft", screenToWorld(glm::vec2(left, top)) },
        { "topRight", screenToWorld(glm::vec2(right, top)) },
        { "bottomLeft", screenToWorld(glm::vec2(left, bottom)) },
        { "bottomRight", screenToWorld(glm::vec2(right, bottom)) },
    };
}

glm::vec2 Camera::worldToScreen(const glm::vec2& world) const
{
    const glm::vec4 clip = (proj * view) * glm::vec4(world, 0.0f, 1.0f);
    const glm::vec2 ndc = glm::vec2(clip) / clip.w;

    return glm::vec2(
        screenRect.x + (ndc.x + 1.0f) * 0.5f * screenRect.z,
        screenRect.y + (1.0f - ndc.y) * 0.5f * screenRect.w
    );
}

void Camera::updateProjection()
{
    const float halfW = 1.0f / zoom;
    const float halfH = halfW / aspect;
    proj = glm::ortho(-halfW, halfW, -halfH, halfH, near, far);
}

void Camera::updateView()
{
    view = glm::translate(glm::mat4(1.0f), glm::vec3(-position, 0.0f));
}

} // namespace kerf