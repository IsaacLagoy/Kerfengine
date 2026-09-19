#include <kerf/kerf.h>
#include <glm/glm.hpp>
#include <iostream>
#include <vector>

using namespace kerf;

namespace {

struct ButtonStyle {
    Button* button = nullptr;
    glm::vec4 idle = glm::vec4(1.0f);
    glm::vec4 hover = glm::vec4(1.0f);
    glm::vec4 down = glm::vec4(1.0f);
};

ButtonStyle makeStyle(Button* button, const glm::vec4& idle)
{
    return ButtonStyle{
        button,
        idle,
        glm::min(idle + glm::vec4(0.18f, 0.18f, 0.18f, 0.0f), glm::vec4(1.0f)),
        glm::max(idle - glm::vec4(0.18f, 0.18f, 0.18f, 0.0f), glm::vec4(0.0f, 0.0f, 0.0f, 1.0f)),
    };
}

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
    Mesh* quad = ObjServer::getMesh("quad");

    Texture* white = TextureServer::getTexture("white");
    Material solid(white);

    const int gridCols = 4;
    const int gridRows = 3;
    const glm::vec2 cellScale(0.14f, 0.14f);
    const float spacingX = 0.36f;
    const float spacingY = 0.36f;
    const float gridOriginX = -((gridCols - 1) * spacingX) * 0.5f;
    const float gridOriginY = ((gridRows - 1) * spacingY) * 0.5f;

    std::vector<ButtonStyle> styles;

    for (int row = 0; row < gridRows; ++row)
    {
        for (int col = 0; col < gridCols; ++col)
        {
            const glm::vec3 pose(
                gridOriginX + col * spacingX,
                gridOriginY - row * spacingY,
                0.0f
            );

            Drop* drop = new Drop(pose, cellScale, quad, &solid, nullptr, Collider2D::box(2.0f, 2.0f));
            drop->setLayer(0.0f);
            drop->setOnUpCallback([row, col](float) {
                std::cout << "drop " << row << "," << col << " up" << std::endl;
            });
            scene.addNode(drop);
            styles.push_back(makeStyle(drop, glm::vec4(0.22f, 0.24f, 0.30f, 1.0f)));
        }
    }

    const glm::vec4 dragColors[] = {
        glm::vec4(0.86f, 0.32f, 0.28f, 1.0f),
        glm::vec4(0.32f, 0.72f, 0.42f, 1.0f),
        glm::vec4(0.32f, 0.55f, 0.90f, 1.0f),
        glm::vec4(0.92f, 0.78f, 0.28f, 1.0f),
    };

    const glm::vec2 dragStart[] = {
        glm::vec2(-1.05f, 0.35f),
        glm::vec2(-1.05f, 0.0f),
        glm::vec2(-1.05f, -0.35f),
        glm::vec2(1.05f, 0.0f),
    };

    for (int i = 0; i < 4; ++i)
    {
        Drag* drag = new Drag(
            glm::vec3(dragStart[i], 0.0f),
            cellScale,
            quad,
            &solid,
            nullptr,
            Collider2D::box(2.0f, 2.0f)
        );
        drag->setLayer(1.0f);
        drag->setOnDownCallback([i](float) { std::cout << "drag " << i << " down" << std::endl; });
        drag->setOnUpCallback([i](float) { std::cout << "drag " << i << " up" << std::endl; });
        scene.addNode(drag);
        styles.push_back(makeStyle(drag, dragColors[i]));
    }

    for (const auto& style : styles)
    {
        style.button->setColor(style.idle);
    }

    while (!engine.shouldClose())
    {
        engine.update();

        for (const auto& style : styles)
        {
            if (style.button->getIsDown())
            {
                style.button->setColor(style.down);
            }
            else if (style.button->getIsHovered())
            {
                style.button->setColor(style.hover);
            }
            else
            {
                style.button->setColor(style.idle);
            }
        }

        engine.render();
    }

    return 0;
}
