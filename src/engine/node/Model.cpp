#include "engine/node/Model.h"
#include "engine/resource/Mesh.h"
#include "engine/resource/ShaderServer.h"
#include "engine/render/material/Material.h"

#include <glm/gtc/type_ptr.hpp>


Model::Model() : Node2d() {}

Model::Model(const glm::vec3& pose, const glm::vec2& scale, Mesh* mesh, Material* material, Shader* shader) :
    Node2d(pose, scale),
    mesh(mesh),
    material(material),
    shader(shader)
{}

Model::~Model() 
{
    unlinkModel();
}

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

void Model::draw(const glm::mat4& viewProjection)
{
    // Text passes nullptr mesh and overrides draw(); skip empty sentinels too.
    if (!mesh) {
        return;
    }

    // Bind here so Text can bind "text" in its override without Scene knowing.
    Shader* shader = this->shader ? this->shader : ShaderServer::getShader("default2d");
    shader->bind();

    // get model matrix
    const glm::mat4& modelMatrix = getModelMatrix();

    GLint loc = shader->getUniformLocation("uViewProjection");
    if (loc >= 0)
    {
        glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(viewProjection));
    }

    // bind model uniforms
    loc = shader->getUniformLocation("uModel");
    if (loc >= 0)
    {
        glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(modelMatrix));
    }

    loc = shader->getUniformLocation("uLayer");
    if (loc >= 0)
    {
        glUniform1f(loc, layer);
    }

    loc = shader->getUniformLocation("uColor");
    if (loc >= 0)
    {
        glUniform4fv(loc, 1, glm::value_ptr(color));
    }

    // bind material uniforms
    loc = shader->getUniformLocation("uAlbedo");
    Texture* albedo = material->getAlbedo();
    if (material && loc >= 0 && albedo) 
    {
        glActiveTexture(GL_TEXTURE0);
        albedo->bind();
        glUniform1i(loc, 0);
    }

    // draw :)
    mesh->draw();
}

void Model::insertModel(Model* pos)
{
    this->nextModel = pos;
    this->prevModel = pos->prevModel;
    pos->prevModel->nextModel = this;
    pos->prevModel = this;
}

void Model::unlinkModel()
{
    if (prevModel) {
        prevModel->nextModel = nextModel;
    }

    if (nextModel) {
        nextModel->prevModel = prevModel;
    }

    nextModel = nullptr;
    prevModel = nullptr;
}