#pragma once

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

// forward declaration
class Camera;

class Mouse {
private:
    GLFWwindow* window; // non-owning pointer to the window

    bool leftMouseUp = false;
    bool leftMouseDown = false;
    bool leftMousePressed = false;
    
    bool rightMouseUp = false;
    bool rightMouseDown = false;
    bool rightMousePressed = false;
    
    bool middleMouseUp = false;
    bool middleMouseDown = false;
    bool middleMousePressed = false;

public:
    Mouse(GLFWwindow* window);
    ~Mouse();

    // update
    void update();

    // position
    glm::vec2 mouseWorld(Camera& camera);

    // button
    bool getLeftDown() const { return leftMouseDown; }
    bool getLeftPressed() const { return leftMousePressed; }
    bool getLeftUp() const { return leftMouseUp; }

    bool getRightDown() const { return rightMouseDown; }
    bool getRightPressed() const { return rightMousePressed; }
    bool getRightUp() const { return rightMouseUp; }

    bool getMiddleDown() const { return middleMouseDown; }
    bool getMiddlePressed() const { return middleMousePressed; }
    bool getMiddleUp() const { return middleMouseUp; }

};