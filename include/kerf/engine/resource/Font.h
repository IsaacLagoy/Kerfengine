#pragma once

#include <string>


namespace kerf {

// forward declarations
class Texture;

/**
 * @brief One baked character in a Font atlas.
 *
 * Metrics are in atlas pixels (the size used when the font was packed).
 * Layout scales them by style.size / Font::pixelHeight.
 * The scene then applies Node::scale to go from layout units to world space.
 */
struct Glyph {
    // How far to move the pen after this character (includes side bearings).
    float advance = 0.0f;

    // Quad corners relative to the pen, y-up to match scene pose.
    float x0 = 0.0f, y0 = 0.0f, x1 = 0.0f, y1 = 0.0f;

    // Atlas UVs for those corners (s,t in [0,1]).
    float u0 = 0.0f, v0 = 0.0f, u1 = 0.0f, v1 = 0.0f;
};

/**
 * @brief Shared rasterized typeface: GPU atlas + CPU glyph metrics.
 *
 * A Font is a resource, not a drawable. Many Text nodes can share one Font.
 * pixelHeight is the bake resolution, not how large text appears in the world.
 *
 * Color, layout size, and string content live on TextStyle / Text, not here,
 * so mixed formatting does not require a new atlas.
 */
class Font {
private:
    // Single-channel (R8) coverage bitmap of packed glyphs.
    Texture* atlas = nullptr;
    int atlasW = 512; // TODO make dynamic? 
    int atlasH = 512;

    // stb raster size: typical capital height in atlas pixels.
    float pixelHeight = 32.0f;

    // Vertical metrics in atlas pixels (ascent above baseline, descent below).
    float ascent = 0.0f;
    float descent = 0.0f;
    float lineGap = 0.0f;

    // ASCII 0-127; entries below 32 are unused. Extend later for Unicode.
    Glyph glyphs[128] {};
    std::string name;

public:
    /**
     * @param name key used by FontServer
     * @param fontPath .ttf path, relative to the executable or absolute
     * @param pixelHeight atlas bake size (quality vs memory), not world height
     */
    Font(const std::string& name, const std::string& fontPath, float pixelHeight);
    ~Font();

    Font(const Font&) = delete;
    Font& operator=(const Font&) = delete;
    Font(Font&&) = delete;
    Font& operator=(Font&&) = delete;

    const Glyph& getGlyph(unsigned char c) const;
    Texture* getAtlas() const { return atlas; }
    float getPixelHeight() const { return pixelHeight; }
    float getAscent() const { return ascent; }
    float getDescent() const { return descent; }
    float getLineGap() const { return lineGap; }
    int getAtlasWidth() const { return atlasW; }
    int getAtlasHeight() const { return atlasH; }
    const std::string& getName() const { return name; }
};

} // namespace kerf