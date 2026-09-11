#pragma once

#include <glm/glm.hpp>


class Camera {
private:
    float aspect;
    float zoom;
    float near = -1000.0f;
    float far = 1000.0f;
    glm::vec2 position = glm::vec2(0.0f);

    float viewportWidth = 1.0f;
    float viewportHeight = 1.0f;

    glm::mat4 proj = glm::mat4(1.0f);
    glm::mat4 view = glm::mat4(1.0f);

public:
    Camera(int viewportWidth, int viewportHeight, float zoom);
    ~Camera();

    void resize(int viewportWidth, int viewportHeight);

    void setAspect(float aspect);
    void setZoom(float zoom);
    void setClipRange(float near, float far);
    void setPosition(const glm::vec2& position);

    glm::mat4 getProjection() const;
    glm::mat4 getView() const;
    glm::vec2 getPosition() const;

    glm::vec2 screenToWorld(const glm::vec2& screen) const;
    glm::vec2 worldToScreen(const glm::vec2& world) const;

private:
    void updateProjection();
    void updateView();
};