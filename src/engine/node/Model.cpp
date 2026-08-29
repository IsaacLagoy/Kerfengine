#include "engine/node/Model.h"
#include "engine/resource/ObjServer.h"
#include "engine/resource/ShaderServer.h"
#include "engine/render/material/Material.h"

#include <glm/gtc/type_ptr.hpp>


Model::Model() : Node(NodeType::MODEL) {}

Model::Model(const glm::vec3& pose, const glm::vec2& scale, Mesh* mesh, Material* material) : Node(pose, scale, NodeType::MODEL), mesh(mesh), material(material) {}

Model::~Model() {}

float Model::getLayer() const
{
    return layer;
}

glm::vec4 Model::getColor() const
{
    return color;
}

Material* Model::getMaterial() const
{
    return material;
}

void Model::setMesh(Mesh* mesh)
{
    this->mesh = mesh;
}

void Model::setMaterial(Material* material)
{
    this->material = material;
}

void Model::setLayer(float layer)
{
    this->layer = layer;
}

void Model::setColor(const glm::vec4& color)
{
    this->color = color;
}

void Model::draw() const
{
    // can only draw a model with a mesh
    if (!mesh) {
        return;
    }

    // get shader for uniform locatiosn
    Shader* shader = ShaderServer::getShader("default2d");

    // bind model uniforms
    glm::mat3 model = getModelMatrix();
    glUniformMatrix3fv(shader->getUniformLocation("uModel"), 1, GL_FALSE, glm::value_ptr(model));
    glUniform1f(shader->getUniformLocation("uLayer"), layer);
    glUniform4fv(shader->getUniformLocation("uColor"), 1, glm::value_ptr(color));

    // bind material uniforms
    if (material) {
        if (Texture* albedo = material->getAlbedo()) {
            glActiveTexture(GL_TEXTURE0);
            albedo->bind();
            glUniform1i(shader->getUniformLocation("uAlbedo"), 0);
        }
    }

    // draw :)
    mesh->draw();
}
