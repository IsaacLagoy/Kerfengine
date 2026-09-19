#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <kerf/engine/render/context/Context.h>
#include <kerf/engine/input/Mouse.h>
#include <kerf/engine/input/Keyboard.h>


namespace kerf {

// forward declarations
class Scene;
class FrameBuffer;

// ------------------------------------------------
// Engine
// ------------------------------------------------

class Engine {
private:
    FrameBuffer* fbo = nullptr;
    Scene* scene = nullptr;
    Context context;
    Mouse mouse;
    Keyboard keyboard;

public:
    Engine(int width, int height);
    ~Engine();

    Engine(const Engine&) = delete;
    Engine& operator=(const Engine&) = delete;
    Engine(Engine&&) = delete;
    Engine& operator=(Engine&&) = delete;

    void setScene(Scene* scene);
    void setFBO(FrameBuffer* fbo);

    Mouse& getMouse();
    Keyboard& getKeyboard();

    /**
     * @brief Draw the scene to the current target (FBO if set, else the window).
     *        Does not swap when an FBO is bound; call present() after.
     */
    void render();

    /**
     * @brief Draw the current FBO to the window (nearest upsample), then swap.
     *        Uses a fullscreen quad so it works with an MSAA backbuffer.
     */
    void present();

    void update();
    bool shouldClose() const;
};

} // namespace kerf