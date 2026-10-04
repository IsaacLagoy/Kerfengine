#pragma once

#include <kerf/engine/render/pipeline/Format.h>

#include <glad/glad.h>
#include <string>
#include <vector>


namespace kerf {

class Texture;

class FrameBuffer {
private:
    GLuint fbo = 0;
    GLuint depthRbo = 0;
    int width = 0;
    int height = 0;
    int colorAttachmentCount = 0;

public:
    struct Attachment {
        std::string name;
        Texture* texture = nullptr;
        Format format = Format::RGBA8;
    };

    FrameBuffer();
    ~FrameBuffer();

    void setAttachments(const std::vector<Attachment>& attachments);

    GLuint getFBO() const;
    int getWidth() const;
    int getHeight() const;

    void bind();
    void unbind();
};

} // namespace kerf
