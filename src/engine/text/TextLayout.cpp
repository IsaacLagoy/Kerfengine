#include "engine/text/TextLayout.h"

#include <algorithm>
#include <utility>
#include <vector>


namespace {

// A single visual line being built (or already finished) during layout.
// Positions inside `chars` are relative to the start of the LINE (x starts
// at 0), not the final position in the box -- that offset is applied later.
struct Line {
    std::vector<LaidOutChar> chars;
    float width = 0.0f;   // pen position after the last glyph (== line width)
    float ascent = 0.0f;  // tallest ascent of any glyph on this line (>= 0)
    float descent = 0.0f; // lowest descent of any glyph on this line (<= 0)
    float gap = 0.0f;     // largest recommended line-gap of any glyph/font on this line
    bool hardBreak = false; // true if this line ended because of an explicit '\n'
                             // (as opposed to a wrap or the end of the text)
};

// Converts a requested point `size` into a multiplier applied to the font's
// baked glyph metrics (which are all authored at `font.getPixelHeight()`).
// e.g. if the atlas was baked at 32px and size=64, everything scales by 2x.
float scaleFor(const Font& font, float size)
{
    const float bake = font.getPixelHeight();
    return (bake > 0.0f) ? (size / bake) : 1.0f;
}

// Grows a line's vertical metrics (ascent/descent/gap) to accommodate a
// glyph rendered at scale `s` from `font`. Since a line's height is driven
// by whichever glyph on it is tallest/lowest, these are running max/min.
void applyLineMetrics(Line& line, const Font& font, float s)
{
    line.ascent = std::max(line.ascent, font.getAscent() * s);
    line.descent = std::min(line.descent, font.getDescent() * s); // descent is negative
    line.gap = std::max(line.gap, font.getLineGap() * s);
}

// Rebuilds ascent/descent/gap for a line from scratch based on the glyphs
// currently in it. Used after a line's glyph list is mutated (e.g. after
// splitting off the tail of a line during word-wrap).
//
// NOTE: this always measures against `font` (the single TextContent font),
// not `ch.font`. That's fine today because a TextLayout only ever uses one
// Font (TextContent::font), but LaidOutChar::font exists per-glyph as if
// mixed fonts were supported -- if that ever changes, this needs updating
// to use each glyph's own font.
void recomputeLineMetrics(Line& line, const Font& font)
{
    line.ascent = 0.0f;
    line.descent = 0.0f;
    line.gap = 0.0f;
    for (const LaidOutChar& ch : line.chars) {
        applyLineMetrics(line, font, ch.displayScale);
    }
}

// Re-walks the glyphs left-to-right and recomputes xStart/xEnd so the line
// starts at pen position 0, preserving each glyph's own advance width.
// Needed whenever a line's glyph list changes without a full re-shape
// (e.g. after slicing off a wrapped tail, whose x values were relative to
// the *old* line and need to be re-based to start at 0 again).
void rebaseLineX(Line& line)
{
    float penX = 0.0f;
    for (LaidOutChar& ch : line.chars) {
        const float w = ch.xEnd - ch.xStart;
        ch.xStart = penX;
        ch.xEnd = penX + w;
        penX = ch.xEnd;
    }
    line.width = penX;
}

// Finds the last space character in a line, searching from the end.
// Used to find where to break a line for word-wrap: everything after
// the space moves down to the next line, the space itself is discarded.
// Returns -1 if there is no space to break at (e.g. one long unbroken word).
int lastSpaceIndex(const Line& line)
{
    for (int i = static_cast<int>(line.chars.size()) - 1; i >= 0; --i) {
        if (line.chars[static_cast<size_t>(i)].c == ' ') {
            return i;
        }
    }
    return -1;
}

// Builds one placed glyph for character `c` at pen position `penX`, using
// `style` (size/color) and `font`'s baked glyph metrics.
LaidOutChar makeGlyph(const Font* font, char c, const TextStyle& style, float penX)
{
    const float s = scaleFor(*font, style.size);
    const Glyph& g = font->getGlyph(static_cast<unsigned char>(c));
    LaidOutChar lc;
    lc.c = c;
    lc.font = font;
    lc.displayScale = s;
    lc.color = style.color;
    lc.xStart = penX;
    lc.xEnd = penX + g.advance * s;      // horizontal "slot" this glyph occupies (for hit-testing)
    lc.baselineY = 0.0f;                  // filled in later, once the line's vertical offset is known
    // g.y0/g.y1 aren't guaranteed to come in top-then-bottom order, so
    // max/min is used to be robust to either winding.
    lc.yTop = std::max(g.y0, g.y1) * s;
    lc.yBottom = std::min(g.y0, g.y1) * s;
    return lc;
}

// Wrap::EVEN ("justify"): stretches the inter-word gaps on this line so its
// right edge lands exactly on `boxWidth`. Only spaces are stretched; glyph
// widths themselves are untouched.
void justifyLine(Line& line, float boxWidth)
{
    int spaces = 0;
    for (const LaidOutChar& ch : line.chars) {
        if (ch.c == ' ') {
            ++spaces;
        }
    }
    if (spaces <= 0) {
        return; // nothing to stretch (e.g. a single word) -- leave as-is
    }
    const float extra = boxWidth - line.width;
    if (extra <= 0.0f) {
        return; // line already fills (or exceeds) the box; nothing to add
    }
    const float each = extra / static_cast<float>(spaces);
    float shift = 0.0f;
    for (LaidOutChar& ch : line.chars) {
        // every glyph after an earlier stretched space needs to move right
        // by the total extra width added so far ("shift")
        ch.xStart += shift;
        ch.xEnd += shift;
        if (ch.c == ' ') {
            ch.xEnd += each; // widen this space's slot...
            shift += each;   // ...and carry the extra width forward
        }
    }
    line.width = boxWidth;
}

// Wrap::CENTER: shifts the whole line right by half of the leftover width
// so it sits centered within `boxWidth`.
void centerLine(Line& line, float boxWidth)
{
    const float extra = boxWidth - line.width;
    if (extra <= 0.0f) {
        return;
    }
    const float dx = 0.5f * extra;
    for (LaidOutChar& ch : line.chars) {
        ch.xStart += dx;
        ch.xEnd += dx;
    }
}

// Baseline-to-baseline distance from this line to the next line below it:
// the space this line's own text occupies (ascent above baseline, plus the
// magnitude of its descent below baseline) plus the font's recommended
// extra line-gap. Descent is negative, hence "ascent - descent" instead of
// "ascent + descent".
float lineStep(const Line& line)
{
    return line.ascent - line.descent + line.gap;
}

// A line with no glyphs (e.g. an empty line from consecutive "\n\n") would
// otherwise have zero height, since ascent/descent are only accumulated
// from glyphs. This gives it "typical" vertical metrics -- based on the
// first span's font size, or 32px if there are no spans at all -- so blank
// lines still take up vertical space like a normal line would.
void fillEmptyLineMetrics(Line& line, const Font& font, const TextContent& content)
{
    // Only fill in defaults if there's nothing real to measure from: either
    // the line has no glyphs, or (defensively) its ascent came out as zero
    // anyway. If it already has real, non-zero metrics, leave them alone.
    if (!line.chars.empty() && line.ascent != 0.0f) {
        return;
    }
    float size = 32.0f;
    if (!content.spans.empty()) {
        size = content.spans.front().style.size;
    }
    applyLineMetrics(line, font, scaleFor(font, size));
}

} // namespace


TextLayout::TextLayout(
    const TextContent& content,
    float maxWidth,
    float maxHeight,
    Wrap wrap,
    [[maybe_unused]] Alignment alignment,
    glm::bvec2 overflow)
{
    if (!content.font) {
        return; // nothing we can shape without a font; layout stays empty
    }
    if (wrap == Wrap::INVALID) {
        wrap = Wrap::NONE;
    }
    if (alignment == Alignment::INVALID) {
        alignment = Alignment::TOP_LEFT;
    }

    const Font* font = content.font;
    // Wrapping only makes sense if there's actually a finite width to wrap against.
    const bool canWrap = (wrap != Wrap::NONE) && maxWidth > 0.0f;
    // "Don't overflow" flags, only meaningful when a bound is actually given.
    const bool clipX = !overflow.x && maxWidth > 0.0f;
    const bool clipY = !overflow.y && maxHeight > 0.0f;

    std::vector<Line> lines; // finished lines, accumulated as we go
    Line line;               // the line currently being built
    bool stop = false;       // set once we must stop laying out more text entirely
                              // (used for both "ran out of vertical room" and
                              // "hit a horizontal clip with no wrapping allowed")

    // What would the total block height be if `next` were appended as the
    // final line after everything already committed in `lines`? Used to
    // decide whether adding one more line would bust maxHeight.
    // Note: unlike lineStep(), this does NOT add `next`'s own line-gap,
    // since a line-gap only matters *between* two lines, not after the
    // last one.
    auto heightIfCommit = [&](const Line& next) {
        float h = 0.0f;
        for (const Line& L : lines) {
            h += lineStep(L);
        }
        h += next.ascent - next.descent;
        return h;
    };

    // Finalizes the in-progress `line`: fills in metrics, re-bases its x
    // positions to start at 0, and either appends it to `lines` or (if it
    // would push the block past maxHeight) discards it and sets `stop`.
    auto commitLine = [&](bool hardBreak) {
        fillEmptyLineMetrics(line, *font, content);
        recomputeLineMetrics(line, *font);
        rebaseLineX(line);
        line.hardBreak = hardBreak;

        // Never reject the very first line even if it alone exceeds
        // maxHeight -- we always want to show at least one line.
        if (clipY && !lines.empty() && heightIfCommit(line) > maxHeight) {
            stop = true;
            line = Line();
            return;
        }

        lines.push_back(std::move(line));
        line = Line();
    };

    // Called when the current line has overflowed maxWidth and wrapping is
    // allowed. Breaks the line at its last space: everything up to (but not
    // including) the space is committed as a finished line, and everything
    // after the space becomes the start of the new current `line`.
    auto tryWrap = [&]() {
        const int space = lastSpaceIndex(line);
        if (space >= 0) {
            Line next;
            next.chars.assign(line.chars.begin() + space + 1, line.chars.end());
            line.chars.resize(static_cast<size_t>(space)); // drops the space itself
            commitLine(false);
            if (stop) {
                return;
            }
            line = std::move(next);
            rebaseLineX(line);           // re-flow tail glyphs starting at x=0
            recomputeLineMetrics(line, *font);
            return;
        }
        // No space anywhere on the line -- it's one long unbreakable run
        // (e.g. a URL or a very long word). Just cut our losses and commit
        // what we have; the character that triggered the overflow will end
        // up starting a fresh line at x=0 (effectively wrapping mid-word).
        if (!line.chars.empty()) {
            commitLine(false);
        }
    };

    // --- Main shaping loop: walk every character of every span in order ---
    for (const TextSpan& span : content.spans) {
        if (stop) {
            break;
        }
        for (char c : span.text) {
            if (stop) {
                break;
            }
            if (c == '\n') {
                commitLine(true); // explicit line break
                continue;
            }
            const unsigned char uc = static_cast<unsigned char>(c);
            if (uc < 32 || uc >= 127) {
                continue; // skip non-printable / non-ASCII characters entirely
            }

            auto pen = [&]() { return line.chars.empty() ? 0.0f : line.chars.back().xEnd; };
            LaidOutChar lc = makeGlyph(font, c, span.style, pen());
            const bool overflowsX = maxWidth > 0.0f && lc.xEnd > maxWidth;

            if (overflowsX && canWrap) {
                // A space that would overflow: just drop it and start a new
                // line (avoids a leading space on the wrapped line).
                if (c == ' ' && !line.chars.empty()) {
                    commitLine(false);
                    continue;
                }
                // A non-space glyph overflowed: try to wrap at the last
                // space in the line, then re-measure this glyph against the
                // new (shorter) line.
                if (!line.chars.empty()) {
                    tryWrap();
                    if (stop) {
                        break;
                    }
                    lc = makeGlyph(font, c, span.style, pen());
                }
                // (if line.chars is empty here, a single glyph is wider
                // than maxWidth by itself -- it's placed anyway below,
                // since there's nothing left to wrap.)
            } else if (overflowsX && clipX) {
                // Not wrapping, and not allowed to overflow horizontally:
                // stop emitting glyphs on this line once one would exceed
                // maxWidth. The glyphs already accumulated on `line` DO fit,
                // so commit them as the final line rather than discarding
                // them -- only the overflowing glyph (and anything after
                // it) is actually clipped away.
                if (line.chars.empty()) {
                    // Guarantee at least one glyph is shown even if it
                    // alone is wider than maxWidth.
                    applyLineMetrics(line, *font, lc.displayScale);
                    line.chars.push_back(lc);
                }
                commitLine(true);
                stop = true;
                break;
            }

            applyLineMetrics(line, *font, lc.displayScale);
            line.chars.push_back(lc);
        }
    }

    // Flush whatever's left in `line` as the final line -- unless we
    // stopped early, in which case whoever set `stop` (the clipY branch in
    // commitLine(), or the clipX branch above) already committed or
    // discarded `line` as appropriate.
    if (!stop) {
        commitLine(true);
    }

    // --- Per-line horizontal treatment for justify / center wrap modes ---
    const float boxW = (maxWidth > 0.0f) ? maxWidth : 0.0f;
    for (size_t i = 0; i < lines.size(); ++i) {
        Line& L = lines[i];
        const bool last = (i + 1 == lines.size());
        // Standard typographic rule: don't justify the last line of a
        // paragraph, or a line that ended on an explicit '\n' (only lines
        // that wrapped because they ran out of room get stretched).
        if (wrap == Wrap::EVEN && boxW > 0.0f && !last && !L.hardBreak) {
            justifyLine(L, boxW);
        } else if (wrap == Wrap::CENTER && boxW > 0.0f) {
            centerLine(L, boxW);
        }
    }

    // --- Stack the lines vertically, assigning each a baseline Y ---
    // Y increases upward here: the first line's baseline is 0, and each
    // subsequent line's baseline is pushed further negative by the
    // previous line's lineStep() (its own height plus line-gap).
    float baseline = 0.0f;
    for (size_t i = 0; i < lines.size(); ++i) {
        Line& L = lines[i];
        if (i > 0) {
            baseline -= lineStep(lines[i - 1]);
        }
        for (LaidOutChar& ch : L.chars) {
            ch.baselineY = baseline;
            ch.yTop += baseline;
            ch.yBottom += baseline;
            chars.push_back(ch);
        }
    }

    if (chars.empty()) {
        return; // nothing placed (empty content, or fully clipped) -- 0x0 layout
    }

    // --- Measure the tight bounding box of everything actually placed ---
    float contentLeft = chars.front().xStart;
    float contentRight = chars.front().xEnd;
    float contentTop = chars.front().yTop;
    float contentBottom = chars.front().yBottom;
    for (const LaidOutChar& ch : chars) {
        contentLeft = std::min(contentLeft, ch.xStart);
        contentRight = std::max(contentRight, ch.xEnd);
        contentTop = std::max(contentTop, ch.yTop);
        contentBottom = std::min(contentBottom, ch.yBottom);
    }

    const float contentW = contentRight - contentLeft;
    const float contentH = contentTop - contentBottom;

    // Pack tight to the content's top-left at (0, 0). Block alignment
    // (BOTTOM / CENTER / RIGHT) is applied later by Textbox against the
    // *final* inner size — aligning here against maxHeight/maxWidth would
    // leave slack in glyph positions while the box still shrinks to content.
    // EVEN/CENTER wrap already placed each line in [0, maxWidth]; keep that.
    const bool wrapOwnsX = (wrap == Wrap::EVEN || wrap == Wrap::CENTER) && maxWidth > 0.0f;
    const float dx = wrapOwnsX ? 0.0f : -contentLeft;
    const float dy = -contentTop;

    for (LaidOutChar& ch : chars) {
        ch.xStart += dx;
        ch.xEnd += dx;
        ch.yTop += dy;
        ch.yBottom += dy;
        ch.baselineY += dy;
    }

    totalWidth = contentW;
    totalHeight = contentH;
}