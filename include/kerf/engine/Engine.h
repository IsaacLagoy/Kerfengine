#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <kerf/engine/render/context/Context.h>
#include <kerf/engine/input/Mouse.h>
#include <kerf/engine/input/Keyboard.h>
#include <memory>


namespace kerf {

class Scene;
class Pipeline;

class Engine {
private:
    Context context;
    Mouse mouse;
    Keyboard keyboard;
    std::unique_ptr<Pipeline> defaultPipeline;
    Pipeline* pipeline = nullptr;
    Scene* scene = nullptr;

public:
    Engine(int width, int height);
    ~Engine();

    Engine(const Engine&) = delete;
    Engine& operator=(const Engine&) = delete;
    Engine(Engine&&) = delete;
    Engine& operator=(Engine&&) = delete;

    void setScene(Scene* scene);
    void setPipeline(Pipeline* pipeline);
    void setClearColor(const glm::vec4& color);
    void setTitle(const std::string& title);

    Mouse& getMouse();
    Keyboard& getKeyboard();

    /**
     * @brief Execute the current pipeline (default: scene → present) and swap.
     */
    void render();

    void update();
    bool shouldClose() const;
};

} // namespace kerf
