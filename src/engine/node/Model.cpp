#include "engine/node/Model.h"
#include "engine/resource/ObjServer.h"
#include "engine/resource/ShaderServer.h"

#include <glm/gtc/type_ptr.hpp>


Model::Model() : Node(NodeType::MODEL) {}

Model::Model(const glm::vec3& pose, const glm::vec2& scale, Mesh* mesh) : Node(pose, scale, NodeType::MODEL), mesh(mesh) {}

Model::~Model() {}

float Model::getLayer() const
{
    return layer;
}

glm::vec4 Model::getColor() const
{
    return color;
}

void Model::setMesh(Mesh* mesh)
{
    this->mesh = mesh;
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
    // get shader for uniform locations
    Shader* shader = ShaderServer::getShader("default2d");

    // set uniforms
    glm::mat3 model = getModelMatrix();
    glUniformMatrix3fv(shader->getUniformLocation("uModel"), 1, GL_FALSE, glm::value_ptr(model));
    glUniform1f(shader->getUniformLocation("uLayer"), layer);
    glUniform4fv(shader->getUniformLocation("uColor"), 1, glm::value_ptr(color));

    // draw
    if (mesh) {
        mesh->draw();
    }
}
