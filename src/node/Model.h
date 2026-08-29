#pragma once

#include "node/Node.h"

// TODO add material

// forward
class Mesh;
class Scene;

class Model : public Node {
friend class Scene;

private:
    Mesh* mesh = nullptr;

    Model* nextModel = nullptr;
    Model* prevModel = nullptr;

    float layer = 0.0f;
    glm::vec4 color = glm::vec4(1.0f);

private:
    // private constructor for scene sentinel nodes
    Model();

public:
    Model(const glm::vec3& pose, const glm::vec2& scale, Mesh* mesh);
    ~Model();

    // getters
    float getLayer() const;
    glm::vec4 getColor() const;

    // setters
    void setMesh(Mesh* mesh);
    void setLayer(float layer);
    void setColor(const glm::vec4& color);

    void draw() const;
};