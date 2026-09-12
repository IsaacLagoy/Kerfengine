#include "engine/node/Textbox.h"
#include "engine/resource/ShaderServer.h"
#include "engine/resource/TextureServer.h"
#include "engine/resource/ObjServer.h"

#include <glm/gtc/type_ptr.hpp>


// Matches shaders/text.vert locations 0, 1, 2.
struct TextVertex {
    float x, y, u, v, r, g, b, a;
};

Textbox::Textbox(
    const TextContent& content,
    const glm::vec3& pose, 
    const glm::vec2& minSize,
    const glm::vec2& maxSize,
    const glm::vec2& padding,
    TextLayout::Wrap wrap,
    TextLayout::Alignment alignment,
    const glm::bvec2& overflow,
    TextLayout::Alignment poseAlignment,
    Shader* shader
) : 
    Model(pose, glm::vec2(1.0f), nullptr, nullptr, shader),
    content(content),
    wrap(wrap),
    alignment(alignment),
    overflow(overflow),
    minSize(minSize),
    maxSize(maxSize),
    padding(padding),
    poseAlignment(poseAlignment)
{
    color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
    initBuffers();
}

Textbox::~Textbox()
{
    destroyBuffers();
}

void Textbox::setContent(const TextContent& content)
{
    this->content = content;
    layoutDirty = true;
}

void Textbox::setWrap(TextLayout::Wrap wrap)
{
    this->wrap = wrap;
    layoutDirty = true;
}

void Textbox::setAlignment(TextLayout::Alignment alignment)
{
    this->alignment = alignment;
    layoutDirty = true;
}

void Textbox::setOverflow(const glm::bvec2& overflow)
{
    this->overflow = overflow;
    layoutDirty = true;
}

void Textbox::setMinSize(const glm::vec2& minSize)
{
    this->minSize = minSize;
    layoutDirty = true;
}

void Textbox::setMaxSize(const glm::vec2& maxSize)
{
    this->maxSize = maxSize;
    layoutDirty = true;
}

void Textbox::setPadding(const glm::vec2& padding)
{
    this->padding = padding;
    layoutDirty = true;
}

void Textbox::setPoseAlignment(TextLayout::Alignment poseAlignment)
{
    this->poseAlignment = poseAlignment;
    // should only shift the mesh, not change the layout: no need to be dirty
}

const TextContent& Textbox::getContent() const
{
    return content;
}

void Textbox::getString(std::string& string) const
{
    std::string text = "";
    for (const TextSpan& span : content.spans) {
        text += span.text;
    }
    string = text;
}

TextLayout::Wrap Textbox::getWrap() const
{
    return wrap;
}

TextLayout::Alignment Textbox::getAlignment() const
{
    return alignment;
}

const glm::bvec2& Textbox::getOverflow() const
{
    return overflow;
}

const glm::vec2& Textbox::getMinSize() const
{
    return minSize;
}

const glm::vec2& Textbox::getMaxSize() const
{
    return maxSize;
}

const glm::vec2& Textbox::getPadding() const
{
    return padding;
}

TextLayout::Alignment Textbox::getPoseAlignment() const
{
    return poseAlignment;
}

void Textbox::draw(const glm::mat4& viewProjection)
{
    // draw background div
    if (color.a > 0.0f)
    {
        // bind shader
        Shader* shader = this->shader ? this->shader : ShaderServer::getShader("default2d");
        shader->bind();

        GLint loc = shader->getUniformLocation("uViewProjection");
        if (loc >= 0)
        {
            glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(viewProjection));
        }

        loc = shader->getUniformLocation("uLayer");
        if (loc >= 0)
        {
            glUniform1f(loc, layer);
        }

        loc = shader->getUniformLocation("uColor");
        if (loc >= 0)
        {
            glUniform4fv(loc, 1, glm::value_ptr(color));
        }

        loc = shader->getUniformLocation("uAlbedo");
        if (loc >= 0)
        {
            glActiveTexture(GL_TEXTURE0);
            TextureServer::getTexture("white")->bind();
            glUniform1i(loc, 0);
        }

        loc = shader->getUniformLocation("uModel");
        if (loc >= 0)
        {
            glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(getTextboxModelMatrix()));
        }

        // draw background
        ObjServer::getMesh("unit")->draw();
    }

    // draw text TODO comment
    if (content.spans.size() > 0)
    {
        rebuildIfDirty();
        if (vertCount == 0) {
            return;
        }

        // bind shader
        Shader* textShader = ShaderServer::getShader("text");
        textShader->bind();

        GLint loc = textShader->getUniformLocation("uViewProjection");
        if (loc >= 0)
        {
            glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(viewProjection));
        }

        loc = textShader->getUniformLocation("uModel");
        if (loc >= 0)
        {
            glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(getTextModelMatrix()));
        }

        loc = textShader->getUniformLocation("uLayer");
        if (loc >= 0)
        {
            glUniform1f(loc, getLayer());
        }

        // Atlas coverage is in .r; blend with scene sprites then restore GL state.
        const GLboolean blendWasEnabled = glIsEnabled(GL_BLEND);
        GLint srcRGB = 0, dstRGB = 0, srcAlpha = 0, dstAlpha = 0;
        glGetIntegerv(GL_BLEND_SRC_RGB, &srcRGB);
        glGetIntegerv(GL_BLEND_DST_RGB, &dstRGB);
        glGetIntegerv(GL_BLEND_SRC_ALPHA, &srcAlpha);
        glGetIntegerv(GL_BLEND_DST_ALPHA, &dstAlpha);

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        loc = textShader->getUniformLocation("uAtlas");
        if (loc >= 0)
        {
            glActiveTexture(GL_TEXTURE0);
            content.font->getAtlas()->bind();
            glUniform1i(loc, 0);
        }

        glBindVertexArray(vao);
        glDrawArrays(GL_TRIANGLES, 0, vertCount);
        glBindVertexArray(0);

        if (!blendWasEnabled) {
            glDisable(GL_BLEND);
        }
        glBlendFuncSeparate(srcRGB, dstRGB, srcAlpha, dstAlpha);
    }

    // draw children
    Node::draw(viewProjection);
}

// TODO comment
void Textbox::initBuffers()
{
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(TextVertex), reinterpret_cast<void*>(0));
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(TextVertex), reinterpret_cast<void*>(offsetof(TextVertex, u)));
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(TextVertex), reinterpret_cast<void*>(offsetof(TextVertex, r)));
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    glBindVertexArray(0);
}

void Textbox::destroyBuffers()
{
    if (vbo) 
    {
        glDeleteBuffers(1, &vbo);
        vbo = 0;
    }

    if (vao) 
    {
        glDeleteVertexArrays(1, &vao);
        vao = 0;
    }
}

void Textbox::rebuildIfDirty()
{
    if (!layoutDirty) {
        return;
    }
    layoutDirty = false;

    // CPU positions, per-glyph color.
    glm::vec2 effMaxSize = getEffectiveMaxSize();
    layout = TextLayout(content, effMaxSize.x, effMaxSize.y, wrap, alignment, overflow);

    // GPU: two triangles per glyph. Color is per-vertex for mixed runs.
    std::vector<TextVertex> verts;
    verts.reserve(layout.chars.size() * 6);

    for (const LaidOutChar& lc : layout.chars) {
        if (!lc.font) {
            continue;
        }

        // get glyph and scale
        const Glyph& g = lc.font->getGlyph(static_cast<unsigned char>(lc.c));
        const float s  = lc.displayScale;
        // Layout space: (0,0) is top-left of the padded content area.
        // Origin → box mapping lives on getTextModelMatrix(), not here.
        const float x0 = lc.xStart + g.x0 * s;
        const float x1 = lc.xStart + g.x1 * s;
        const float y0 = lc.baselineY + g.y0 * s;
        const float y1 = lc.baselineY + g.y1 * s;
        const float r  = lc.color.r;
        const float gv = lc.color.g;
        const float b  = lc.color.b;
        const float a  = lc.color.a;

        // add vertices (the two triangles)
        verts.push_back({ x0, y0, g.u0, g.v0, r, gv, b, a });
        verts.push_back({ x1, y0, g.u1, g.v0, r, gv, b, a });
        verts.push_back({ x1, y1, g.u1, g.v1, r, gv, b, a });

        verts.push_back({ x0, y0, g.u0, g.v0, r, gv, b, a });
        verts.push_back({ x1, y1, g.u1, g.v1, r, gv, b, a });
        verts.push_back({ x0, y1, g.u0, g.v1, r, gv, b, a });
    }

    // upload to GPU
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(verts.size() * sizeof(TextVertex)), verts.data(), GL_DYNAMIC_DRAW);
    glBindVertexArray(0);

    // update vertex count
    vertCount = static_cast<int>(verts.size());
}

// TODO cache this computation or smth
glm::mat4 Textbox::getBaseModelMatrix() const
{
    // compute effective box scale
    glm::vec2 size = getEffectiveSize();
    glm::vec2 half = 0.5f * size;

    // compute pose based on alignment
    glm::vec2 boxPos(0.0f);
    switch (poseAlignment) 
    {
        case TextLayout::Alignment::TOP_LEFT: boxPos = glm::vec2(half.x, -half.y); break;
        case TextLayout::Alignment::TOP_RIGHT: boxPos = glm::vec2(-half.x, -half.y); break;
        case TextLayout::Alignment::BOTTOM_LEFT: boxPos = glm::vec2(half.x, half.y); break;
        case TextLayout::Alignment::BOTTOM_RIGHT: boxPos = glm::vec2(-half.x, half.y); break;
        case TextLayout::Alignment::CENTER: break;
        default: throw std::invalid_argument("Invalid pose alignment");
    }

    // alignment point should be the pivot at getPose().
    // use scale 1 so this is a pure translation
    glm::mat4 model, transModel;
    Node::computeModelMatrix(transModel, glm::vec3(boxPos.x, boxPos.y, 0.0f), glm::vec2(1.0f));
    Node::computeModelMatrix(model, getPose(), glm::vec2(1.0f));
    return model * transModel;
}

glm::mat4 Textbox::getTextModelMatrix() const
{
    const glm::vec2 size = getEffectiveSize();
    const glm::vec2 half = 0.5f * size;
    const glm::vec2 inner = size - 2.0f * padding;

    // Layout is packed tight at the padded top-left. Shift the block inside
    // the inner rect (the sized box, not maxSize).
    glm::vec2 extra(0.0f);
    const bool wrapOwnsX =
        (wrap == TextLayout::Wrap::EVEN || wrap == TextLayout::Wrap::CENTER) &&
        inner.x > 0.0f;
    switch (alignment) {
        case TextLayout::Alignment::TOP_LEFT:
            break;
        case TextLayout::Alignment::TOP_RIGHT:
            if (!wrapOwnsX) {
                extra.x = inner.x - layout.totalWidth;
            }
            break;
        case TextLayout::Alignment::BOTTOM_LEFT:
            extra.y = -(inner.y - layout.totalHeight);
            break;
        case TextLayout::Alignment::BOTTOM_RIGHT:
            if (!wrapOwnsX) {
                extra.x = inner.x - layout.totalWidth;
            }
            extra.y = -(inner.y - layout.totalHeight);
            break;
        case TextLayout::Alignment::CENTER:
            if (!wrapOwnsX) {
                extra.x = 0.5f * (inner.x - layout.totalWidth);
            }
            extra.y = -0.5f * (inner.y - layout.totalHeight);
            break;
        default:
            break;
    }

    const glm::vec3 origin(
        -half.x + padding.x + extra.x,
        half.y - padding.y + extra.y,
        0.0f);
    glm::mat4 local;
    Node::computeModelMatrix(local, origin, glm::vec2(1.0f));
    return getBaseModelMatrix() * local;
}

glm::mat4 Textbox::getTextboxModelMatrix() const
{
    glm::vec2 size = getEffectiveSize();
    glm::mat4 local;
    Node::computeModelMatrix(local, glm::vec3(0.0f), size);
    return getBaseModelMatrix() * local;
}

glm::vec2 Textbox::getEffectiveSize() const
{
    glm::vec2 size = glm::max(minSize, glm::vec2(layout.totalWidth + 2.0f * padding.x, layout.totalHeight + 2.0f * padding.y));
    size = glm::min(size, glm::max(maxSize, glm::vec2(0.0f)));
    return size;
}

glm::vec2 Textbox::getEffectiveMaxSize() const
{
    return maxSize - 2.0f * padding;
}
