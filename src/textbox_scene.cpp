#include "engine/Engine.h"
#include "engine/render/camera/Camera.h"
#include "engine/scene/Scene.h"
#include "engine/resource/FontServer.h"
#include "engine/node/Textbox.h"
#include "engine/text/TextLayout.h"
#include "engine/node/Model.h"
#include "engine/resource/TextureServer.h"
#include "engine/render/material/Material.h"
#include "engine/resource/ObjServer.h"

int main() 
{
    // create scene
    const int width = 800;
    const int height = 600;
    Engine engine(width, height);

    Camera camera(width, height, 1.0f);
    Scene scene;
    scene.setCamera(&camera);
    engine.setScene(&scene);

    // load font
    FontServer::loadFont("ui", "resources/font/Arial.ttf",32.0f); 
    Font* font = FontServer::getFont("ui");

    for (int i = 0; i < 1; i++) {
        TextStyle style;
        style.size = 0.1f;
        style.color = glm::vec4(0, 0, 0, 1.0f);
        Textbox* textbox = new Textbox(
            TextContent{ font, { TextSpan{ "Hello, World! NO! NO! NO! NO! NO! NO! NO! NO! NO! NO! NO! NO! NO! NO! NO! NO! NO! NO! NO! NO! NO! NO! NO! NO! NO! NO! NO! NO! NO! NO! NO! NO! NO! NO! NO! NO! NO! NO! NO! NO! NO! NO! NO! NO! NO!", style } } },
            glm::vec3(0.0f, 0.0f, 0.0f),
            glm::vec2(2.0f, 0.5f),
            glm::vec2(2.0f, 1.0f),
            glm::vec2(0.1f, 0.1f),
            TextLayout::Wrap::EDGE,
            TextLayout::Alignment::TOP_LEFT,
            glm::bvec2(false),
            TextLayout::Alignment::CENTER
        );
        textbox->setColor(glm::vec4(0.5f + 0.05f * i, 1.0f, 1.0f, 1.0f));
        scene.addNode(textbox);
    }

    Texture* white = TextureServer::getTexture("white");
    Material solid(white);

    Model* unit = new Model(glm::vec3(-0.5f, 1.0f, 0.0f), glm::vec2(1.0f, 1.0f), ObjServer::getMesh("unit"), &solid);
    scene.addNode(unit);

    while (!engine.shouldClose()) {
        engine.update();
        engine.render();
    }

    return 0;
}