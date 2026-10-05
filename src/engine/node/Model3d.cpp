#include <kerf/engine/node/Model3d.h>
#include <kerf/engine/resource/ShaderServer.h>
#include <kerf/engine/resource/Mesh.h>
#include <kerf/engine/render/material/Material.h>
#include <glm/gtc/type_ptr.hpp>

namespace kerf {

// ------------------------------------------------------------
// Constructors
// ------------------------------------------------------------

Model3d::Model3d() : Node3d() {}

Model3d::Model3d(const glm::vec3& position, const glm::quat& rotation, const glm::vec3& scale, Mesh* mesh, Material* material, Shader* shader) : 
    Node3d(position, rotation, scale), 
    AbstractModel(mesh, material, shader)
{}

void Model3d::draw(const glm::mat4& viewProjection)
{
    const glm::mat4& modelMatrix = getModelMatrix();
    AbstractModel::draw(viewProjection, modelMatrix);
    Node::draw(viewProjection);
}

} // namespace kerf