#pragma once 

#include <glm/glm.hpp>
#include <unordered_map>
#include <variant>
#include <string>
#include <optional>


namespace kerf {

// forward declarations
class Mesh;
class Material;
class Shader;

// ------------------------------------------------------------
// Uniforms
// ------------------------------------------------------------

using UniformType = std::variant<
    bool,
    uint8_t,
    uint16_t,
    uint32_t,
    int8_t,
    int16_t,
    int32_t,
    float, 
    glm::vec2,
    glm::vec3,
    glm::vec4,
    glm::mat4,
    glm::mat3,
    glm::mat2,
    glm::quat
>;

// ------------------------------------------------------------
// AbstractModel
// ------------------------------------------------------------

class AbstractModel {
private:
    Mesh* mesh = nullptr;
    Material* material = nullptr;
    Shader* shader = nullptr;
    glm::vec4 color = glm::vec4(1.0f);

    std::unordered_map<std::string, UniformType> uniforms;

public:
    AbstractModel();
    AbstractModel(Mesh* mesh, Material* material, Shader* shader);
    ~AbstractModel() = default;

    void draw(
        const glm::mat4& viewProjection,
        const glm::mat4& modelMatrix,
        std::optional<float> layer = std::nullopt
    );

    // uniforms
    void setUniform(const std::string& name, const UniformType& value);
    void removeUniform(const std::string& name);

    // getters
    Mesh* getMesh() const;
    Material* getMaterial() const;
    Shader* getShader() const;
    glm::vec4 getColor() const;

    // setters
    void setMesh(Mesh* mesh);
    void setMaterial(Material* material);
    void setShader(Shader* shader);
    void setColor(const glm::vec4& color);

    static void uploadUniform(Shader* shader, const std::string& name, const UniformType& value);
};

}; // namespace kerf