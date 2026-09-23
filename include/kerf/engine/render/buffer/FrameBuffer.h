#pragma once

#include <glad/glad.h>
#include <string>
#include <vector>


namespace kerf {

class Texture;

class FrameBuffer {
private:
    GLuint fbo = 0;
    GLuint depthRbo = 0;
    Texture* texture = nullptr;
    int width = 0;
    int height = 0;
    int colorAttachmentCount = 0;

public:
    struct Attachment {
        std::string name;
        Texture* texture = nullptr;
    };

    FrameBuffer();
    ~FrameBuffer();

    void setTexture(Texture* texture, int width, int height);
    void setColor(Texture* texture);
    void setAttachments(const std::vector<Attachment>& attachments);

    GLuint getFBO() const;
    int getWidth() const;
    int getHeight() const;
    Texture* getTexture() const;

    void bind();
    void unbind();
};

} // namespace kerf
