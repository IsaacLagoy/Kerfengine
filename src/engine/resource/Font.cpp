#include <kerf/engine/resource/Font.h>
#include <kerf/engine/resource/Texture.h>

#include "shared/Const.h"
#include "shared/Files.h"

// stb_truetype implementation lives in this translation unit only.
#define STB_TRUETYPE_IMPLEMENTATION
#include <stb_truetype.h>

#include <fstream>
#include <stdexcept>
#include <vector>


namespace kerf {
    
Font::Font(const std::string& name, const std::string& fontPath, float pixelHeight) :
    pixelHeight(pixelHeight),
    name(name)
{
    // ------------------------------------------------
    // 1. Load the .ttf into memory (stb needs the whole file)
    // ------------------------------------------------
    const std::string resolved = Files::resolvePath(fontPath);

    std::ifstream file(resolved, std::ios::binary | std::ios::ate);
    if (!file) {
        throw std::runtime_error(ANSI_RED + "[Font] could not open " + resolved + ANSI_RESET);
    }

    const size_t size = static_cast<size_t>(file.tellg());
    file.seekg(0);
    std::vector<unsigned char> ttfBuffer(size);
    file.read(reinterpret_cast<char*>(ttfBuffer.data()), static_cast<std::streamsize>(size));

    stbtt_fontinfo info;
    if (!stbtt_InitFont(&info, ttfBuffer.data(), 0)) {
        throw std::runtime_error(ANSI_RED + "[Font] stbtt_InitFont failed for " + resolved + ANSI_RESET);
    }

    // ------------------------------------------------
    // 2. Rasterize printable ASCII into one atlas bitmap
    //    Oversampling = extra samples per pixel for sharper small text.
    // ------------------------------------------------
    atlasW = 512; // TODO remove hardcoded value
    atlasH = 512;
    std::vector<unsigned char> atlasBitmap(static_cast<size_t>(atlasW * atlasH), 0);

    stbtt_pack_context packCtx;
    stbtt_PackBegin(&packCtx, atlasBitmap.data(), atlasW, atlasH, 0, 1, nullptr);
    stbtt_PackSetOversampling(&packCtx, 2, 2);

    stbtt_packedchar packedChars[95]; // glyphs 32..126
    stbtt_PackFontRange(&packCtx, ttfBuffer.data(), 0, pixelHeight, 32, 95, packedChars);
    stbtt_PackEnd(&packCtx);

    // ------------------------------------------------
    // 3. Font-wide vertical metrics, converted from font units to atlas pixels
    // ------------------------------------------------
    int rawAscent = 0, rawDescent = 0, rawLineGap = 0;
    stbtt_GetFontVMetrics(&info, &rawAscent, &rawDescent, &rawLineGap);
    const float scale = stbtt_ScaleForPixelHeight(&info, pixelHeight);
    ascent = static_cast<float>(rawAscent) * scale;
    descent = static_cast<float>(rawDescent) * scale;
    lineGap = static_cast<float>(rawLineGap) * scale;

    // ------------------------------------------------
    // 4. Per-glyph quads + UVs. stb y grows down from the pen;
    //    negate Y so layout/draw match the engine's y-up pose.
    // ------------------------------------------------
    for (int c = 32; c < 127; ++c) {
        stbtt_aligned_quad q;
        float dummyX = 0.0f, dummyY = 0.0f;
        stbtt_GetPackedQuad(packedChars, atlasW, atlasH, c - 32, &dummyX, &dummyY, &q, 0);

        Glyph& g = glyphs[c];
        g.x0 = q.x0;
        g.x1 = q.x1;
        g.y0 = -q.y0;
        g.y1 = -q.y1;
        g.u0 = q.s0;
        g.v0 = q.t0;
        g.u1 = q.s1;
        g.v1 = q.t1;
        g.advance = packedChars[c - 32].xadvance;
    }

    // Coverage only: fragment shader reads .r as alpha.
    atlas = new Texture(atlasW, atlasH, GL_R8, GL_RED, GL_UNSIGNED_BYTE);
    atlas->setPixels(atlasW, atlasH, atlasBitmap.data());
}

Font::~Font()
{
    delete atlas;
    atlas = nullptr;
}

const Glyph& Font::getGlyph(unsigned char c) const
{
    return glyphs[c];
}

} // namespace kerf