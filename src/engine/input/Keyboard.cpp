#include <kerf/engine/input/Keyboard.h>


namespace kerf {

Keyboard::Keyboard(GLFWwindow* window) : window(window) {}

Keyboard::~Keyboard() {}

void Keyboard::update() {
    for (int i = 0; i < GLFW_KEY_LAST + 1; i++) 
    {
        bool wasPressed = keysPressed[i];

        keysPressed[i] = glfwGetKey(window, i) == GLFW_PRESS;
        keysDown[i] = keysPressed[i] && !wasPressed;
        keysUp[i] = !keysPressed[i] && wasPressed;
    }
}

} // namespace kerf