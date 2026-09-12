#include "engine/Engine.h"
#include "engine/node/Button.h"
#include "engine/physics/collision/Collider2D.h"
#include "engine/physics/collision/ColliderPolygon2DMesh.h"
#include "engine/render/camera/Camera.h"
#include "engine/render/material/Material.h"
#include "engine/resource/ObjServer.h"
#include "engine/resource/TextureServer.h"
#include "engine/scene/Scene.h"

#include <iostream>
#include <vector>

namespace {

struct ButtonStyle {
    Button* button = nullptr;
    glm::vec4 idle = glm::vec4(1.0f);
    glm::vec4 hover = glm::vec4(1.0f);
    glm::vec4 down = glm::vec4(1.0f);
};

} // namespace

int main()
{
    const int width = 800;
    const int height = 600;
    Engine engine(width, height);

    Camera camera(width, height, 1.0f);
    Scene scene;
    scene.setCamera(&camera);
    engine.setScene(&scene);

    ObjServer::loadMesh("quad", "resources/mesh/quad.obj");
    ObjServer::loadMesh("octagon", "resources/mesh/octagon.obj");
    Mesh* quad = ObjServer::getMesh("quad");
    Mesh* octagon = ObjServer::getMesh("octagon");

    ColliderPolygon2DMeshServer::loadMesh("octagon", octagon->getVertices());

    Texture* white = TextureServer::getTexture("white");
    Material solid(white);

    Button* quadButton = new Button(
        glm::vec3(-0.45f, 0.0f, 0.0f),
        glm::vec2(0.25f, 0.25f),
        quad,
        &solid,
        nullptr,
        Collider2D::box(2.0f, 2.0f)
    );
    quadButton->setLayer(0.0f);
    quadButton->setOnDownCallback([](float) { std::cout << "quad button down" << std::endl; });
    quadButton->setOnUpCallback([](float) { std::cout << "quad button up" << std::endl; });
    scene.addNode(quadButton);

    Button* octagonButton = new Button(
        glm::vec3(0.45f, 0.0f, 1.0f),
        glm::vec2(0.25f, 0.25f),
        octagon,
        &solid,
        nullptr,
        Collider2D::polygon(ColliderPolygon2DMeshServer::getMesh("octagon"))
    );
    octagonButton->setLayer(0.0f);
    octagonButton->setOnDownCallback([](float) { std::cout << "octagon button down" << std::endl; });
    octagonButton->setOnUpCallback([](float) { std::cout << "octagon button up" << std::endl; });
    scene.addNode(octagonButton);

    std::vector<ButtonStyle> buttons = {
        { quadButton,
          glm::vec4(0.35f, 0.55f, 0.85f, 1.0f),
          glm::vec4(0.50f, 0.70f, 0.95f, 1.0f),
          glm::vec4(0.20f, 0.35f, 0.65f, 1.0f) },
        { octagonButton,
          glm::vec4(0.85f, 0.45f, 0.30f, 1.0f),
          glm::vec4(0.95f, 0.60f, 0.40f, 1.0f),
          glm::vec4(0.65f, 0.25f, 0.15f, 1.0f) },
    };

    for (const auto& style : buttons) 
    {
        style.button->setColor(style.idle);
    }

    while (!engine.shouldClose())
    {
        engine.update();

        const bool keyDown = engine.getKeyboard().getDown(GLFW_KEY_SPACE);
        if (keyDown) {
            std::cout << "space key down" << std::endl;
        }

        const glm::vec2 mouse = engine.getMouse().mouseWorld(camera);
        const bool mouseDown = engine.getMouse().getLeftPressed();
        for (const auto& style : buttons)
        {
            style.button->update(0.016f, mouse, mouseDown);
            if (style.button->getIsDown()) {
                style.button->setColor(style.down);
            } else if (style.button->getIsHovered()) {
                style.button->setColor(style.hover);
            } else {
                style.button->setColor(style.idle);
            }
        }

        engine.render();
    }

    return 0;
}
