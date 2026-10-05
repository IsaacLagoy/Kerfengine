#include <kerf/engine/node/Model.h>
#include <kerf/engine/resource/Mesh.h>
#include <kerf/engine/resource/ShaderServer.h>
#include <kerf/engine/render/material/Material.h>
#include <glm/gtc/type_ptr.hpp>


namespace kerf {

// ------------------------------------------------------------
// Constructors
// ------------------------------------------------------------
    
Model::Model() : Node2d() {}

Model::Model(const glm::vec3& pose, const glm::vec2& scale, Mesh* mesh, Material* material, Shader* shader) :
    Node2d(pose, scale),
    AbstractModel(mesh, material, shader)
{}

void Model::setLayer(float layer)
{
    this->layer = layer;
}

float Model::getLayer() const
{
    return layer;
}

void Model::draw(const glm::mat4& viewProjection)
{
    const glm::mat4& modelMatrix = getModelMatrix();
    AbstractModel::draw(viewProjection, modelMatrix, layer);
    Node::draw(viewProjection);
}

} // namespace kerf