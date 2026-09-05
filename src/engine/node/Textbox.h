#pragma once

#include "engine/node/Model.h"
#include "engine/text/TextLayout.h"

#include <glad/glad.h>
#include <string>


class Textbox : public Model {
public:
    

private:
    TextContent content;

    // formatting variables
    TextLayout::Wrap wrap = TextLayout::Wrap::INVALID;

    // the alignment of the text within the textbox
    TextLayout::Alignment alignment = TextLayout::Alignment::INVALID;

    // whether to overflow the textbox if the text is too long
    // does nothing if there is no max size
    glm::bvec2 overflow = glm::bvec2(true);

    glm::vec2 minSize = glm::vec2(0.0f);
    glm::vec2 maxSize = glm::vec2(-1.0f);
    glm::vec2 padding = glm::vec2(0.0f);

    // the slignment of the textbox itself relative to the pose
    TextLayout::Alignment poseAlignment = TextLayout::Alignment::INVALID;

    // layout variables
    TextLayout layout;
    bool layoutDirty = true;

    // dynamic glyph mesh: pos.xy, uv, color. Not a Mesh (wrong vertex layout).
    GLuint vao = 0;
    GLuint vbo = 0;
    int vertCount = 0;

public:
    Textbox(
        const TextContent& content,
        const glm::vec3& pose, 
        const glm::vec2& minSize,
        const glm::vec2& maxSize,
        const glm::vec2& padding,
        TextLayout::Wrap wrap,
        TextLayout::Alignment alignment,
        const glm::bvec2& overflow,
        TextLayout::Alignment poseAlignment
    );
    ~Textbox() override;

    // disable copying and moving
    Textbox(const Textbox&) = delete;
    Textbox& operator=(const Textbox&) = delete;
    Textbox(Textbox&&) = delete;
    Textbox& operator=(Textbox&&) = delete;

    // setters
    void setContent(const TextContent& content);
    void setWrap(TextLayout::Wrap wrap);
    void setAlignment(TextLayout::Alignment alignment);
    void setOverflow(const glm::bvec2& overflow);
    void setMinSize(const glm::vec2& minSize);
    void setMaxSize(const glm::vec2& maxSize);
    void setPadding(const glm::vec2& padding);
    void setPoseAlignment(TextLayout::Alignment poseAlignment);

    // getters
    const TextContent& getContent() const;
    void getString(std::string& string) const;
    TextLayout::Wrap getWrap() const;
    TextLayout::Alignment getAlignment() const;
    const glm::bvec2& getOverflow() const;
    const glm::vec2& getMinSize() const;
    const glm::vec2& getMaxSize() const;
    const glm::vec2& getPadding() const;
    TextLayout::Alignment getPoseAlignment() const;

    void draw(const glm::mat4& viewProjection) override;

private:
    void initBuffers();
    void destroyBuffers();
    void rebuildIfDirty();
    glm::mat3 getBaseModelMatrix() const;
    glm::mat3 getTextModelMatrix() const;
    glm::mat3 getTextboxModelMatrix() const;
    glm::vec2 getEffectiveSize() const;
    glm::vec2 getEffectiveMaxSize() const;
};