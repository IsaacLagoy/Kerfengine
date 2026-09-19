#include <kerf/engine/input/Mouse.h>
#include <kerf/engine/render/camera/Camera.h>


namespace kerf {

Mouse::Mouse(GLFWwindow* window) : window(window) {}

Mouse::~Mouse() {}

glm::vec2 Mouse::mouseWorld(Camera& camera) {
    double mx = 0.0;
    double my = 0.0;
    glfwGetCursorPos(window, &mx, &my);

    int winW = 0;
    int winH = 0;
    int fbW = 0;
    int fbH = 0;
    glfwGetWindowSize(window, &winW, &winH);
    glfwGetFramebufferSize(window, &fbW, &fbH);

    camera.resize(fbW, fbH);

    if (winW <= 0 || winH <= 0) 
    {
        return glm::vec2(0.0f);
    }

    const glm::vec2 screen(
        static_cast<float>(mx * static_cast<double>(fbW) / static_cast<double>(winW)),
        static_cast<float>(my * static_cast<double>(fbH) / static_cast<double>(winH))
    );
    return camera.screenToWorld(screen);
}

void Mouse::update() {
    bool wasLeftPressed = leftMousePressed;
    bool wasRightPressed = rightMousePressed;
    bool wasMiddlePressed = middleMousePressed;

    leftMousePressed = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;
    rightMousePressed = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS;
    middleMousePressed = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_MIDDLE) == GLFW_PRESS;

    leftMouseDown = leftMousePressed && !wasLeftPressed;
    rightMouseDown = rightMousePressed && !wasRightPressed;
    middleMouseDown = middleMousePressed && !wasMiddlePressed;

    leftMouseUp = !leftMousePressed && wasLeftPressed;
    rightMouseUp = !rightMousePressed && wasRightPressed;
    middleMouseUp = !middleMousePressed && wasMiddlePressed;
}

} // namespace kerf