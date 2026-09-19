#pragma once

#include <kerf/engine/node/Node3d.h>


namespace kerf {
    
// forward declarations
class Mesh;
class Material;
class Scene;
class Shader;

class Model3d : public Node3d {
    friend class Scene;
    
private:
    Mesh* mesh;
    Material* material;
    Shader* shader;

    glm::vec4 color = glm::vec4(1.0f);

public:
    Model3d(const glm::vec3& position, const glm::quat& rotation, const glm::vec3& scale, Mesh* mesh, Material* material, Shader* shader);
    ~Model3d();

    // getters
    glm::vec4 getColor() const;
    Material* getMaterial() const;

    // setters
    void setMesh(Mesh* mesh);
    void setMaterial(Material* material);
    void setColor(const glm::vec4& color);

protected:
    // protected constructor for scene sentinel nodes
    Model3d();

    virtual void draw(const glm::mat4& viewProjection) override;
};

} // namespace kerf