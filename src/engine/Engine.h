#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "engine/render/context/Context.h"


class Scene;
class FrameBuffer;


class Engine {
private:
    int width = 800;
    int height = 600;
    FrameBuffer* fbo = nullptr;

    Scene* scene = nullptr;
    Context context;

public:
    Engine(int width, int height);
    ~Engine();

    Engine(const Engine&) = delete;
    Engine& operator=(const Engine&) = delete;
    Engine(Engine&&) = delete;
    Engine& operator=(Engine&&) = delete;

    void setScene(Scene* scene);
    void setFBO(FrameBuffer* fbo);

    void render();
    void update();
    bool shouldClose() const;
};