#pragma once // internal header

#include <kerf/engine/resource/Font.h>

#include <glm/glm.hpp>
#include <string>
#include <vector>


namespace kerf {

 struct TextStyle {
    float size = 32.0f;
    glm::vec4 color = glm::vec4(1.0f);
};

struct TextSpan {
    std::string text;
    TextStyle style;
};

struct TextContent {
    Font* font = nullptr;
    std::vector<TextSpan> spans;
};


/**
 * @brief one placed glyph after layout. Used for drawing and hit-testing.
 *
 * positions are in layout units (atlas pixels scaled by displayScale).
 * For Textbox those units are world-sized local coords (no Node::scale).
 */
 struct LaidOutChar {
    char c = '\0';
    const Font* font = nullptr;

    // horizontal slot of this character (pen advance), for hit-test.
    float xStart = 0.0f;
    float xEnd = 0.0f;

    // glyph ink bounds (not the same as the advance slot).
    float yTop = 0.0f;
    float yBottom = 0.0f;

    float baselineY = 0.0f;
    float displayScale = 1.0f;
    glm::vec4 color = glm::vec4(1.0f);
};

/**
 * @brief CPU result of shaping a string. No OpenGL.
 *
 * Cached on Text and rebuilt only when the string/style/box is dirty.
 * Hit-testing and measuring can use this without a GPU upload.
 */
class TextLayout {
public:
    enum class Wrap {
        INVALID,
        NONE,
        EDGE,
        EVEN,
        CENTER
    };

    enum class Alignment {
        INVALID,
        TOP_LEFT,
        TOP_RIGHT,
        BOTTOM_LEFT,
        BOTTOM_RIGHT,
        CENTER
    };

    std::vector<LaidOutChar> chars;
    float totalWidth = 0.0f;
    float totalHeight = 0.0f;

    TextLayout() = default;

    /**
     * @param maxWidth  inner content width (Textbox: maxSize.x - 2*padding). <= 0 means unbounded.
     * @param maxHeight inner content height (Textbox: maxSize.y - 2*padding). <= 0 means unbounded.
     * @param overflow  if false, do not emit glyphs past maxWidth / maxHeight. Ignored when that max is unbounded.
     *
     * Block alignment is applied by Textbox against the final inner size.
     * Padding, minSize, and poseAlignment stay on Textbox.
     */
    TextLayout(
        const TextContent& content,
        float maxWidth = -1.0f,
        float maxHeight = -1.0f,
        Wrap wrap = Wrap::NONE,
        Alignment alignment = Alignment::TOP_LEFT,
        glm::bvec2 overflow = glm::bvec2(true)
    );
};

} // namespace kerf

 // TODO add hit test in model space later
 // Just implement having text for now