#pragma once

#include <kerf/engine/node/Node2d.h>
#include <kerf/engine/render/material/AbstractModel.h>
#include <glm/glm.hpp>


namespace kerf {
    
// forward declarations
class Mesh;
class Scene;
class Material;
class Shader;

class Model : public Node2d, public AbstractModel {
    friend class Scene;

protected:
    float layer = 0.0f;

public:
    Model(const glm::vec3& pose, const glm::vec2& scale, Mesh* mesh, Material* material, Shader* shader);
    ~Model() = default;

    // setters
    void setLayer(float layer);

    // getters
    float getLayer() const;

protected:
    // protected constructor for scene sentinel nodes
    Model();

    virtual void draw(const glm::mat4& viewProjection) override;    
};

} // namespace kerf