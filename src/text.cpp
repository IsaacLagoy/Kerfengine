// text_render_example.cpp
//
// Minimal stb_truetype-based text rendering for OpenGL (3.3 core).
// Assumes GLFW window + glad already initialized elsewhere.
//
// Get stb_truetype.h from: https://github.com/nothings/stb
//
// Build (example): g++ text_render_example.cpp glad.c -lglfw -ldl -o demo

#include <glad/glad.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

#include "engine/resource/ShaderServer.h"

#include <algorithm>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <exception>
#include <fstream>
#include <string>
#include <vector>

// ---------------------------------------------------------------------
// Font atlas
// ---------------------------------------------------------------------

struct Glyph {
    float advance;          // how far to move pen after drawing this glyph
    float x0, y0, x1, y1;   // quad offset from baseline/pen (pixels)
    float u0, v0, u1, v1;   // atlas UV coords
};

struct Font {
    GLuint texture = 0;
    int atlasW = 512, atlasH = 512;
    float pixelHeight = 32.0f;
    float ascent, descent, lineGap;
    Glyph glyphs[128]; // ASCII 0-127; extend to a map for Unicode
};

static bool loadFont(const char* path, float pixelHeight, Font& font) {
    // 1. Read font file into memory
    std::ifstream f(path, std::ios::binary | std::ios::ate);
    if (!f) { std::fprintf(stderr, "Could not open font %s\n", path); return false; }
    size_t size = (size_t)f.tellg();
    f.seekg(0);
    std::vector<unsigned char> ttfBuffer(size);
    f.read((char*)ttfBuffer.data(), size);

    stbtt_fontinfo info;
    if (!stbtt_InitFont(&info, ttfBuffer.data(), 0)) {
        std::fprintf(stderr, "stbtt_InitFont failed\n");
        return false;
    }

    // 2. Pack glyphs into a bitmap atlas (stbtt_pack gives better AA than BakeFontBitmap)
    font.pixelHeight = pixelHeight;
    font.atlasW = 512;
    font.atlasH = 512;
    std::vector<unsigned char> atlasBitmap(font.atlasW * font.atlasH);

    stbtt_pack_context packCtx;
    stbtt_PackBegin(&packCtx, atlasBitmap.data(), font.atlasW, font.atlasH, 0, 1, nullptr);
    stbtt_PackSetOversampling(&packCtx, 2, 2); // subpixel oversampling for nicer small text

    stbtt_packedchar packedChars[95]; // ASCII 32..126
    stbtt_PackFontRange(&packCtx, ttfBuffer.data(), 0, pixelHeight, 32, 95, packedChars);
    stbtt_PackEnd(&packCtx);

    // 3. Font-wide vertical metrics (in font units -> scale to pixels)
    int ascent, descent, lineGap;
    stbtt_GetFontVMetrics(&info, &ascent, &descent, &lineGap);
    float scale = stbtt_ScaleForPixelHeight(&info, pixelHeight);
    font.ascent  = ascent  * scale;
    font.descent = descent * scale;
    font.lineGap = lineGap * scale;

    // 4. Convert per-glyph pack data into our Glyph struct (normalized UVs, quad offsets)
    for (int c = 32; c < 127; ++c) {
        stbtt_aligned_quad q;
        float dummyX = 0, dummyY = 0;
        stbtt_GetPackedQuad(packedChars, font.atlasW, font.atlasH,
                             c - 32, &dummyX, &dummyY, &q, 0);

        Glyph& g = font.glyphs[c];
        g.x0 = q.x0; g.y0 = q.y0; g.x1 = q.x1; g.y1 = q.y1; // already pen-relative
        g.u0 = q.s0; g.v0 = q.t0; g.u1 = q.s1; g.v1 = q.t1;
        g.advance = packedChars[c - 32].xadvance;
    }

    // 5. Upload atlas to GL as a single-channel (red) texture
    glGenTextures(1, &font.texture);
    glBindTexture(GL_TEXTURE_2D, font.texture);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, font.atlasW, font.atlasH, 0,
                 GL_RED, GL_UNSIGNED_BYTE, atlasBitmap.data());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    return true;
}

// ---------------------------------------------------------------------
// Layout cache: built once per string/box, reused for both rendering
// and hit-testing. This is the piece that answers "which char is at x?".
// ---------------------------------------------------------------------

struct LaidOutChar {
    char c;
    float xStart, xEnd;   // horizontal extent of this char's advance slot (pen space)
    float yTop, yBottom;  // for multi-line boxes; single-line here for simplicity
};

struct TextLayout {
    std::vector<LaidOutChar> chars;
    float totalWidth = 0.0f;
};

// Lay out a single line of text starting at pen (0,0). Handles simple
// left-to-right clipping to maxWidth (truncates, no wrapping) as an
// example of the "cutoff" behavior discussed earlier.
static TextLayout layoutText(const Font& font, const std::string& text, float maxWidth = -1.0f) {
    TextLayout layout;
    float penX = 0.0f;

    for (char c : text) {
        if (c < 32 || c >= 127) continue;
        const Glyph& g = font.glyphs[(unsigned char)c];

        float xStart = penX;
        float xEnd   = penX + g.advance;

        if (maxWidth > 0 && xEnd > maxWidth) break; // simple cutoff, no ellipsis

        layout.chars.push_back({ c, xStart, xEnd, 0.0f, font.pixelHeight });
        penX = xEnd;
    }
    layout.totalWidth = penX;
    return layout;
}

// Binary search the layout for the character index under a given local-space x.
// Returns index into layout.chars, or (int)chars.size() if past the end.
// "local space" = mouse position already transformed into text-box coordinates
// (i.e. subtract box origin and any scroll offset before calling this).
static int hitTestX(const TextLayout& layout, float localX) {
    const auto& chars = layout.chars;
    if (chars.empty() || localX < chars.front().xStart) return 0;
    if (localX >= layout.totalWidth) return (int)chars.size();

    auto it = std::lower_bound(
        chars.begin(), chars.end(), localX,
        [](const LaidOutChar& ch, float x) { return ch.xEnd <= x; });

    int idx = (int)(it - chars.begin());

    // Decide "before" or "after" the character based on midpoint,
    // for natural text-cursor placement (click left half -> before char,
    // right half -> after char).
    const LaidOutChar& ch = chars[idx];
    float mid = (ch.xStart + ch.xEnd) * 0.5f;
    return (localX < mid) ? idx : idx + 1;
}

// ---------------------------------------------------------------------
// Rendering: batch all glyph quads for a layout into one VBO, draw once.
// ---------------------------------------------------------------------

struct Vertex { float x, y, u, v; };

// Build one VBO's worth of vertices for a laid-out line of text.
static std::vector<Vertex> buildVertices(const Font& font, const TextLayout& layout,
                                          const std::string& text, float baselineY) {
    std::vector<Vertex> verts;
    verts.reserve(layout.chars.size() * 6);

    for (const auto& lc : layout.chars) {
        const Glyph& g = font.glyphs[(unsigned char)lc.c];
        float x0 = lc.xStart + g.x0, x1 = lc.xStart + g.x1;
        float y0 = baselineY + g.y0, y1 = baselineY + g.y1;

        // two triangles per glyph quad
        verts.push_back({x0, y0, g.u0, g.v0});
        verts.push_back({x1, y0, g.u1, g.v0});
        verts.push_back({x1, y1, g.u1, g.v1});

        verts.push_back({x0, y0, g.u0, g.v0});
        verts.push_back({x1, y1, g.u1, g.v1});
        verts.push_back({x0, y1, g.u0, g.v1});
    }
    return verts;
}

// Column-major ortho. Origin at top-left, y grows down, units in pixels.
static void orthoTopLeft(float width, float height, float out[16]) {
    const float l = 0.0f, r = width, t = 0.0f, b = height;
    const float n = -1.0f, f = 1.0f;
    out[0]  = 2.0f / (r - l); out[4]  = 0.0f;           out[8]  = 0.0f;            out[12] = -(r + l) / (r - l);
    out[1]  = 0.0f;           out[5]  = 2.0f / (t - b); out[9]  = 0.0f;            out[13] = -(t + b) / (t - b);
    out[2]  = 0.0f;           out[6]  = 0.0f;           out[10] = -2.0f / (f - n); out[14] = -(f + n) / (f - n);
    out[3]  = 0.0f;           out[7]  = 0.0f;           out[11] = 0.0f;            out[15] = 1.0f;
}

static void framebufferSizeCallback(GLFWwindow*, int width, int height) {
    glViewport(0, 0, width, height);
}

int main() {
    if (!glfwInit()) {
        std::fprintf(stderr, "glfwInit failed\n");
        return 1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);

    const int winW = 800, winH = 600;
    GLFWwindow* window = glfwCreateWindow(winW, winH, "Text", nullptr, nullptr);
    if (!window) {
        std::fprintf(stderr, "glfwCreateWindow failed\n");
        glfwTerminate();
        return 1;
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::fprintf(stderr, "gladLoadGLLoader failed\n");
        glfwTerminate();
        return 1;
    }

    int fbW = 0, fbH = 0;
    glfwGetFramebufferSize(window, &fbW, &fbH);
    glViewport(0, 0, fbW, fbH);

    Font font;
    const char* fontPath = "/System/Library/Fonts/Supplemental/Arial.ttf";
    if (!loadFont(fontPath, 32.0f, font)) {
        glfwTerminate();
        return 1;
    }

    try {
        ShaderServer::loadShader("text", "shaders/text.vert", "shaders/text.frag");
    } catch (const std::exception& e) {
        std::fprintf(stderr, "%s\n", e.what());
        glfwTerminate();
        return 1;
    }
    Shader* textShader = ShaderServer::getShader("text");

    GLuint vao, vbo;
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, u));
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    std::string text = "Hello, OpenGL text!";
    float boxOriginX = 50, boxOriginY = 50, boxMaxWidth = 300;

    TextLayout layout = layoutText(font, text, boxMaxWidth);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glClearColor(0.12f, 0.12f, 0.14f, 1.0f);

    int prevClick = GLFW_RELEASE;
    while (!glfwWindowShouldClose(window)) {
        int w = 0, h = 0;
        glfwGetWindowSize(window, &w, &h);

        int click = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT);
        if (click == GLFW_PRESS && prevClick == GLFW_RELEASE) {
            double mx, my;
            glfwGetCursorPos(window, &mx, &my);
            float localX = (float)mx - boxOriginX;
            int charIndex = hitTestX(layout, localX);
            std::printf("Cursor would go before char index %d\n", charIndex);
        }
        prevClick = click;

        glClear(GL_COLOR_BUFFER_BIT);
        textShader->bind();

        float proj[16];
        orthoTopLeft((float)w, (float)h, proj);

        glUniformMatrix4fv(textShader->getUniformLocation("uProjection"), 1, GL_FALSE, proj);
        glUniform2f(textShader->getUniformLocation("uOrigin"), boxOriginX, boxOriginY);
        glUniform3f(textShader->getUniformLocation("uColor"), 1.0f, 1.0f, 1.0f);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, font.texture);
        glUniform1i(textShader->getUniformLocation("uAtlas"), 0);

        auto verts = buildVertices(font, layout, text, font.ascent);
        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(Vertex), verts.data(), GL_DYNAMIC_DRAW);
        glDrawArrays(GL_TRIANGLES, 0, (GLsizei)verts.size());

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteBuffers(1, &vbo);
    glDeleteVertexArrays(1, &vao);
    ShaderServer::clear();
    glDeleteTextures(1, &font.texture);
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
