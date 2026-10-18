#include <kerf/kerf.h>
#include <glm/glm.hpp>
#include <algorithm>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

using namespace kerf;

// DropAccordion playground.
//
//   bottom: horizontal accordion (the "hand")     right: vertical accordion
//   left:   a plain Drop and a StickyDrop          middle: two loose cards
//   bottom: StickyDropAccordion: cards released outside a target fly back to their slot
//
// The window title shows the bottom row's getOrder(); the pending card is in (parens).
//
// Keys:
//   =  add a card to the bottom row        -  remove the bottom row's last card
//   R  rotate the bottom row 15 degrees    L  lock / unlock the bottom row
//   [  slower follow                       ]  faster follow
//   C  toggle bottom row capacity (unlimited / 6)

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

const glm::vec4 kCardColors[] = {
    glm::vec4(0.86f, 0.32f, 0.28f, 1.0f),
    glm::vec4(0.32f, 0.72f, 0.42f, 1.0f),
    glm::vec4(0.32f, 0.55f, 0.90f, 1.0f),
    glm::vec4(0.92f, 0.78f, 0.28f, 1.0f),
    glm::vec4(0.70f, 0.42f, 0.86f, 1.0f),
    glm::vec4(0.95f, 0.55f, 0.25f, 1.0f),
    glm::vec4(0.30f, 0.80f, 0.80f, 1.0f),
    glm::vec4(0.90f, 0.45f, 0.65f, 1.0f),
};

const glm::vec4 kTargetColor(0.22f, 0.24f, 0.30f, 1.0f);
const glm::vec4 kLockedColor(0.42f, 0.18f, 0.18f, 1.0f);

} // namespace

int main()
{
    const int width = 800;
    const int height = 600;
    Engine engine(width, height);
    const Target sceneColor(TargetSize::fromPixels(width, height), Format::RGBA8, Filter::Linear, "uColor");

    Camera camera(width, height, 0.5f);
    Scene scene;
    scene.setCamera(&camera);
    engine.setScene(&scene);

    ObjServer::loadMesh("quad", "resources/mesh/quad.obj");
    Mesh* quad = ObjServer::getMesh("quad");

    Texture* white = TextureServer::getTexture("white");
    Material solid(white);

    const Collider2D box = Collider2D::box(2.0f, 2.0f);
    const glm::vec2 cardScale(0.08f, 0.11f);

    std::vector<ButtonStyle> styles;
    std::unordered_map<const Drag*, int> cardIds;
    int nextCardId = 0;

    auto logEvents = [](DropBase* target, const std::string& name) {
        target->setOnDropCallback([name](float) { std::cout << name << ": drop" << std::endl; });
        target->setOnPickupCallback([name](float) { std::cout << name << ": pickup" << std::endl; });
    };

    // ------------------------------------------------------------
    // targets (added first so they draw under the cards)
    // ------------------------------------------------------------

    auto* handParent = new Node2d(
        glm::vec3(0.0f, 2.0f, 0.1f),
        glm::vec2(0.25, 2.0f)
    );
    scene.addNode(handParent);

    auto* hand = new DropAccordion(glm::vec3(0.0f, -0.7f, 0.0f), glm::vec2(0.9f, 0.14f), quad, &solid, nullptr, box);
    hand->setLayer(0.0f);
    handParent->addChild(hand);
    logEvents(hand, "hand");

    // a column is just a rotated accordion: -90 degrees puts index 0 at the top
    auto* column = new DropAccordion(glm::vec3(1.1f, 0.15f, glm::radians(-90.0f)), glm::vec2(0.55f, 0.12f), quad, &solid, nullptr, box);
    column->setLayer(0.0f);
    scene.addNode(column);
    logEvents(column, "column");

    auto* drop = new Drop(glm::vec3(-1.1f, 0.45f, 0.0f), glm::vec2(0.12f, 0.15f), quad, &solid, nullptr, box);
    drop->setLayer(0.0f);
    scene.addNode(drop);
    logEvents(drop, "drop");

    auto* sticky = new StickyDrop(glm::vec3(-1.1f, 0.0f, 0.0f), glm::vec2(0.12f, 0.15f), quad, &solid, nullptr, box, 0.4f);
    sticky->setLayer(0.0f);
    scene.addNode(sticky);
    logEvents(sticky, "sticky");

    auto* stickyRow = new StickyDropAccordion(glm::vec3(0.0f, -0.9f, 0.0f), glm::vec2(0.7f, 0.14f), quad, &solid, nullptr, box, 0.4f);
    stickyRow->setLayer(0.0f);
    scene.addNode(stickyRow);
    logEvents(stickyRow, "sticky row");

    for (Button* target : std::initializer_list<Button*>{hand, column, drop, sticky, stickyRow})
    {
        styles.push_back(makeStyle(target, kTargetColor));
    }

    // ------------------------------------------------------------
    // cards
    // ------------------------------------------------------------

    auto makeCard = [&](const glm::vec2& at) {
        const int id = nextCardId++;
        Drag* card = new Drag(glm::vec3(at, 0.0f), cardScale, quad, &solid, nullptr, box);
        card->setLayer(1.0f);
        scene.addNode(card);
        cardIds[card] = id;
        styles.push_back(makeStyle(card, kCardColors[id % 8]));
        return card;
    };

    for (int i = 0; i < 5; ++i)
    {
        hand->insertDrag(makeCard(glm::vec2(0.0f)));
    }
    for (int i = 0; i < 3; ++i)
    {
        column->insertDrag(makeCard(glm::vec2(0.0f)));
    }
    for (int i = 0; i < 3; ++i)
    {
        stickyRow->insertDrag(makeCard(glm::vec2(0.0f)));
    }
    drop->setDrag(makeCard(glm::vec2(drop->getWorldPose())));
    sticky->setDrag(makeCard(glm::vec2(sticky->getWorldPose())));
    makeCard(glm::vec2(-0.3f, 0.3f));
    makeCard(glm::vec2(0.3f, 0.3f));

    std::cout << "Keys: = add card, - remove card, R rotate, L lock, [ ] follow speed, C capacity" << std::endl;

    // ------------------------------------------------------------
    // loop
    // ------------------------------------------------------------

    std::string lastTitle;
    while (!engine.shouldClose())
    {
        engine.update();
        Keyboard& keys = engine.getKeyboard();

        if (keys.getDown(GLFW_KEY_EQUAL))
        {
            Drag* card = makeCard(glm::vec2(hand->getWorldPose()));
            if (!hand->insertDrag(card))
            {
                std::cout << "hand refused the new card (locked or full)" << std::endl;
            }
        }
        if (keys.getDown(GLFW_KEY_MINUS) && hand->getCount() > 0)
        {
            Drag* last = hand->getDrag(hand->getCount() - 1);
            if (scene.getSelectedDrag() != last)
            {
                styles.erase(std::remove_if(styles.begin(), styles.end(),
                    [last](const ButtonStyle& s) { return s.button == last; }), styles.end());
                cardIds.erase(last);
                delete last;
            }
        }
        if (keys.getDown(GLFW_KEY_R))
        {
            const glm::vec3 pose = hand->getPose();
            hand->setPose(glm::vec3(pose.x, pose.y, pose.z + glm::radians(15.0f)));
        }
        if (keys.getDown(GLFW_KEY_L))
        {
            hand->setLocked(!hand->isLocked());
            std::cout << "hand " << (hand->isLocked() ? "locked" : "unlocked") << std::endl;
        }
        if (keys.getDown(GLFW_KEY_LEFT_BRACKET) || keys.getDown(GLFW_KEY_RIGHT_BRACKET))
        {
            const float factor = keys.getDown(GLFW_KEY_RIGHT_BRACKET) ? 1.5f : 1.0f / 1.5f;
            hand->setFollowSpeed(hand->getFollowSpeed() * factor);
            column->setFollowSpeed(hand->getFollowSpeed());
            stickyRow->setFollowSpeed(hand->getFollowSpeed());
            std::cout << "follow speed " << hand->getFollowSpeed() << std::endl;
        }
        if (keys.getDown(GLFW_KEY_C))
        {
            hand->setCapacity(hand->getCapacity() < 0 ? 6 : -1);
            std::cout << "hand capacity " << (hand->getCapacity() < 0 ? std::string("unlimited") : std::to_string(hand->getCapacity())) << std::endl;
        }

        // hand order in the title, pending card in (parens)
        std::string title = "hand [";
        for (Drag* card : hand->getOrder())
        {
            const std::string id = std::to_string(cardIds[card]);
            title += (card == hand->getPending() ? "(" + id + ")" : id) + " ";
        }
        if (title.back() == ' ')
        {
            title.pop_back();
        }
        title += "]";
        if (title != lastTitle)
        {
            engine.setTitle(title);
            std::cout << title << std::endl;
            lastTitle = title;
        }

        for (const auto& style : styles)
        {
            if (style.button == hand && hand->isLocked())
            {
                style.button->setColor(kLockedColor);
            }
            else if (style.button->getIsDown())
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

        engine.scenePass(sceneColor);
        engine.present(sceneColor, PresentFit::Letterbox);
    }

    return 0;
}
