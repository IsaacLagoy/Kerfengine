#pragma once

#include <kerf/engine/node/Node3d.h>
#include <kerf/engine/render/material/AbstractModel.h>


namespace kerf {
    
// forward declarations
class Mesh;
class Material;
class Scene;
class Shader;

class Model3d : public Node3d, public AbstractModel {
    friend class Scene;

public:
    Model3d(const glm::vec3& position, const glm::quat& rotation, const glm::vec3& scale, Mesh* mesh, Material* material, Shader* shader);
    ~Model3d() = default;

protected:
    // protected constructor for scene sentinel nodes
    Model3d();

    virtual void draw(const glm::mat4& viewProjection) override;
};

} // namespace kerf