#pragma once

#include "engine/node/Node2d.h"

#include <glm/glm.hpp>

// forward declarations
class Mesh;
class Scene;
class Material;

class Model : public Node2d {
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

protected:
    // protected constructor for scene sentinel nodes
    Model();

    void insertModel(Model* pos);
    void unlinkModel();
    virtual void draw(const glm::mat4& viewProjection) override;    
};