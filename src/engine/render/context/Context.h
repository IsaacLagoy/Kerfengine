#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <string>


/**
 * @brief Handles internal context for standalone applications
 * 
 */
class Context {
private:
    // 
    GLFWwindow* window;

public:
    /**
     * @brief Construct a new Context object
     * 
     * @param width 
     * @param height 
     * @param resizable 
     */
    Context(int width, int height, bool resizable=true);

    /**
     * @brief Destroy the Context object
     * 
     */
    ~Context();

    /**
     * @brief Swaps the GLFW window buffers displaying the back buffer to the screen
     * 
     */
    void swapBuffers() const;

    /**
     * @brief Get whether GLFW think the window should close. Primarily used for closing the window vis its x button. 
     * 
     * @return true 
     * @return false 
     */
    bool shouldClose() const;

    /**
     * @brief resizes the window to the given size.
     * 
     * @param width 
     * @param height 
     */
    void resize(int width, int height) const;

    /**
     * @brief Get the Window object
     * 
     * @return GLFWwindow* 
     */
    GLFWwindow* getWindow() const;

    /**
     * @brief Set the Title object
     * 
     * @param title 
     */
    void setTitle(const std::string& title) const;

private:
    /**
     * @brief Creates the context window with GLFW
     * 
     * @param width 
     * @param height 
     * @param resizable 
     */
    void initWindow(int width, int height, bool resizable);

    /**
     * @brief Sets the window as the current OpenGL context
     * 
     */
    void initGL() const;

};