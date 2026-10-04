#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <kerf/engine/render/context/Context.h>
#include <kerf/engine/input/Mouse.h>
#include <kerf/engine/input/Keyboard.h>
#include <kerf/engine/render/pipeline/Target.h>
#include <initializer_list>
#include <memory>
#include <string>


namespace kerf {

class Scene;
class Shader;
class RenderSystem;

class Engine {
private:
    Context context;
    Mouse mouse;
    Keyboard keyboard;
    std::unique_ptr<RenderSystem> renderer;
    Scene* scene = nullptr;
    double previousTime = 0.0;
    float deltaTime = 0.0f;
    bool hasPreviousTime = false;

public:
    Engine(int width, int height);
    ~Engine();

    Engine(const Engine&) = delete;
    Engine& operator=(const Engine&) = delete;
    Engine(Engine&&) = delete;
    Engine& operator=(Engine&&) = delete;

    void setScene(Scene* scene);
    void setClearColor(const glm::vec4& color);
    void setTitle(const std::string& title);

    Mouse& getMouse();
    Keyboard& getKeyboard();
    float getDeltaTime() const;

    void scenePass(Target output);
    void postPass( Shader* shader, std::initializer_list<Target> inputs, Target output);
    void present( Target color, PresentFit fit = PresentFit::Stretch, Filter filter = Filter::Linear);

    void update();
    bool shouldClose() const;
private:
    void framebufferSize(int& width, int& height) const;

};

} // namespace kerf
