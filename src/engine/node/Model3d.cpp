#include "engine/node/Model3d.h"
#include "engine/resource/ShaderServer.h"
#include "engine/resource/Mesh.h"
#include "engine/render/material/Material.h"

#include <glm/gtc/type_ptr.hpp>


// ------------------------------------------------------------
// Constructors
// ------------------------------------------------------------

Model3d::Model3d() : Node3d() {}

Model3d::Model3d(const glm::vec3& position, const glm::quat& rotation, const glm::vec3& scale, Mesh* mesh, Material* material, Shader* shader)
    : Node3d(position, rotation, scale), mesh(mesh), material(material), shader(shader)
{}

Model3d::~Model3d()
{
    // no op
}

glm::vec4 Model3d::getColor() const
{
    return color;
}

Material* Model3d::getMaterial() const
{
    return material;
}

void Model3d::setMesh(Mesh* mesh)
{
    this->mesh = mesh;
}

void Model3d::setMaterial(Material* material)
{
    this->material = material;
}

void Model3d::setColor(const glm::vec4& color)
{
    this->color = color;
}

void Model3d::draw(const glm::mat4& viewProjection)
{
    if (!mesh) {
        return;
    }

    Shader* shader = this->shader ? this->shader : ShaderServer::getShader("default3d");
    shader->bind();

    const glm::mat4& modelMatrix = getModelMatrix();

    GLint loc = shader->getUniformLocation("uViewProjection");
    if (loc >= 0)
    {
        glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(viewProjection));
    }

    loc = shader->getUniformLocation("uModel");
    if (loc >= 0)
    {
        glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(modelMatrix));
    }

    loc = shader->getUniformLocation("uColor");
    if (loc >= 0)
    {
        glUniform4fv(loc, 1, glm::value_ptr(color));
    }

    loc = shader->getUniformLocation("uAlbedo");
    Texture* albedo = material ? material->getAlbedo() : nullptr;
    if (loc >= 0 && albedo)
    {
        glActiveTexture(GL_TEXTURE0);
        albedo->bind();
        glUniform1i(loc, 0);
    }

    // draw :)
    mesh->draw();

    // draw children
    Node::draw(viewProjection);
}