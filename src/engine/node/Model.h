#pragma once

#include "engine/node/Node.h"

#include <glm/glm.hpp>

// forward declarations
class Mesh;
class Scene;
class Material;

class Model : public Node {
    
friend class Scene;

protected:
    Mesh* mesh = nullptr;
    Material* material = nullptr;

    Model* nextModel = nullptr;
    Model* prevModel = nullptr;

    float layer = 0.0f;

    // color multiplier applied to the material color
    glm::vec4 color = glm::vec4(1.0f);

public:
    Model(const glm::vec3& pose, const glm::vec2& scale, Mesh* mesh, Material* material);
    ~Model();

    // getters
    float getLayer() const;
    glm::vec4 getColor() const;
    Material* getMaterial() const;

    // setters
    void setMesh(Mesh* mesh);
    void setMaterial(Material* material);
    void setLayer(float layer);
    void setColor(const glm::vec4& color);

    // Scene does not bind a program. Each drawable binds its shader here.
    virtual void draw(const glm::mat4& viewProjection);

protected:
    // private constructor for scene sentinel nodes
    Model();

protected:
    explicit Model(NodeType type);
    Model(const glm::vec3& pose, const glm::vec2& scale, Mesh* mesh, Material* material, NodeType type);

    void insertModel(Model* pos);
    void unlinkModel();
};