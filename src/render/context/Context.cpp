#include "Context.h"

#include <stdexcept>
#include <iostream>


Context::Context(int width, int height, bool resizable)
{
    initWindow(width, height, resizable);
    initGL();
}

Context::~Context()
{
    if (window) glfwDestroyWindow(window);
    glfwTerminate();
}

void Context::swapBuffers() const
{
    glfwSwapBuffers(window);
}

bool Context::shouldClose() const
{
    return glfwWindowShouldClose(window);
}

void Context::resize(int width, int height) const
{
    glfwSetWindowSize(window, width, height);
    glViewport(0, 0, width, height);
}

GLFWwindow* Context::getWindow() const 
{
    return window;
}

void Context::setTitle(const std::string& title) const
{
    glfwSetWindowTitle(window, title.c_str());
}

void Context::initWindow(int width, int height, bool resizable)
{
    if (!glfwInit()) 
    {
        throw std::runtime_error("Failed to initialize GLFW");
    }

    // increase depth buffer precision
    glfwWindowHint(GLFW_DEPTH_BITS, 24);

    // enable MSAA
    glfwWindowHint(GLFW_SAMPLES, 4);

    // set window resizable
    glfwWindowHint(GLFW_RESIZABLE, resizable);

    // request version 3.3
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // required on macOS

    // attempt to create the window
    window = glfwCreateWindow(width, height, "Terrain", nullptr, nullptr);
    if (!window)
    { 
        glfwTerminate();
        throw std::runtime_error("Failed to open window");
    }

    // set window to current context
    glfwMakeContextCurrent(window);
    glfwSetTime(0.0);
}

void Context::initGL() const
{
    if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress)))
    {
        throw std::runtime_error("Failed to load GLAD");
    }

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    std::cout << "OpenGL version: " << glGetString(GL_VERSION) << std::endl;
    std::cout << "GLSL version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;
}

