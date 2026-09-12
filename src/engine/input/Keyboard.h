#pragma once

#include <array>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

class Keyboard {
private:
    GLFWwindow* window;

    std::array<bool, GLFW_KEY_LAST + 1> keysPressed;
    std::array<bool, GLFW_KEY_LAST + 1> keysDown;
    std::array<bool, GLFW_KEY_LAST + 1> keysUp;

public:
    Keyboard(GLFWwindow* window);
    ~Keyboard();

    void update();

    bool getPressed(int key) const { return keysPressed[key]; }
    bool getDown(int key) const { return keysDown[key]; }
    bool getUp(int key) const { return keysUp[key]; }
};