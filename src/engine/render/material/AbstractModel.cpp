#include <kerf/engine/render/material/AbstractModel.h>
#include <kerf/engine/render/material/Material.h>
#include <kerf/engine/resource/ShaderServer.h>
#include <kerf/engine/resource/ObjServer.h>
#include <kerf/engine/resource/Texture.h>
#include <glm/gtc/type_ptr.hpp>

#include <type_traits>


namespace kerf {

AbstractModel::AbstractModel() :
    mesh(nullptr),
    material(nullptr),
    shader(nullptr),
    color(glm::vec4(1.0f))
{}

AbstractModel::AbstractModel(Mesh* mesh, Material* material, Shader* shader) :
    mesh(mesh),
    material(material),
    shader(shader),
    color(glm::vec4(1.0f))
{}

void AbstractModel::draw(
    const glm::mat4& viewProjection,
    const glm::mat4& modelMatrix,
    std::optional<float> layer
) {
    if (!getMesh()) {
        return;
    }

    bool is2d = layer.has_value();

    // bind shader 
    Shader* shader;
    if (getShader()) 
    {
        shader = getShader();
    } 
    else if (is2d) 
    {
        shader = ShaderServer::getShader("default2d");
    } 
    else 
    {
        shader = ShaderServer::getShader("default3d");
    }
    shader->bind();


    // upload uniforms
    uploadUniform(shader, "uViewProjection", viewProjection);
    uploadUniform(shader, "uModel", modelMatrix);
    uploadUniform(shader, "uColor", color);

    if (is2d) 
    {
        uploadUniform(shader, "uLayer", layer.value());
    }

    // bind material uniforms
    Texture* albedo = getMaterial() ? getMaterial()->getAlbedo() : nullptr;
    if (albedo) 
    {
        glActiveTexture(GL_TEXTURE0);
        albedo->bind();
        uploadUniform(shader, "uAlbedo", int32_t{0});
    }

    // upload uniforms
    for (const auto& [name, value] : uniforms)
    {
        uploadUniform(shader, name, value);
    }

    // draw :)
    getMesh()->draw();
}

void AbstractModel::setUniform(const std::string& name, const UniformType& value)
{
    uniforms[name] = value;
}

void AbstractModel::removeUniform(const std::string& name)
{
    uniforms.erase(name);
}

// getters
Mesh* AbstractModel::getMesh() const
{
    return mesh;
}

Material* AbstractModel::getMaterial() const
{
    return material;
}

Shader* AbstractModel::getShader() const
{
    return shader;
}

glm::vec4 AbstractModel::getColor() const
{
    return color;
}

// setters
void AbstractModel::setMesh(Mesh* mesh)
{
    this->mesh = mesh;
}

void AbstractModel::setMaterial(Material* material)
{
    this->material = material;
}

void AbstractModel::setShader(Shader* shader)
{
    this->shader = shader;
}

void AbstractModel::setColor(const glm::vec4& color)
{
    this->color = color;
}

void AbstractModel::uploadUniform(Shader* shader, const std::string& name, const UniformType& value)
{
    GLint loc = shader->getUniformLocation(name);
    if (loc < 0)
    {
        return;
    }

    std::visit([&](const auto& arg) {
        using T = std::decay_t<decltype(arg)>;
        if constexpr (std::is_same_v<T, bool>) {
            glUniform1i(loc, arg ? 1 : 0);
        } else if constexpr (std::is_same_v<T, uint8_t> || std::is_same_v<T, uint16_t>) {
            glUniform1ui(loc, static_cast<GLuint>(arg));
        } else if constexpr (std::is_same_v<T, uint32_t>) {
            glUniform1ui(loc, arg);
        } else if constexpr (std::is_same_v<T, int8_t> || std::is_same_v<T, int16_t>) {
            glUniform1i(loc, static_cast<GLint>(arg));
        } else if constexpr (std::is_same_v<T, int32_t>) {
            glUniform1i(loc, arg);
        } else if constexpr (std::is_same_v<T, float>) {
            glUniform1f(loc, arg);
        } else if constexpr (std::is_same_v<T, glm::vec2>) {
            glUniform2fv(loc, 1, glm::value_ptr(arg));
        } else if constexpr (std::is_same_v<T, glm::vec3>) {
            glUniform3fv(loc, 1, glm::value_ptr(arg));
        } else if constexpr (std::is_same_v<T, glm::vec4>) {
            glUniform4fv(loc, 1, glm::value_ptr(arg));
        } else if constexpr (std::is_same_v<T, glm::mat4>) {
            glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(arg));
        } else if constexpr (std::is_same_v<T, glm::mat3>) {
            glUniformMatrix3fv(loc, 1, GL_FALSE, glm::value_ptr(arg));
        } else if constexpr (std::is_same_v<T, glm::mat2>) {
            glUniformMatrix2fv(loc, 1, GL_FALSE, glm::value_ptr(arg));
        } else if constexpr (std::is_same_v<T, glm::quat>) {
            glUniform4fv(loc, 1, glm::value_ptr(arg));
        }
    }, value);
}

}; // namespace kerf
