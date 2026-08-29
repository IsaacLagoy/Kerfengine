#pragma once

#include <glad/glad.h>
#include <string>
#include <unordered_map>
#include <memory>
#include <string>


// ------------------------------------------------------------
// Shader class
// ------------------------------------------------------------

/**
 * @brief OpenGL shader program
 * 
 * Creates and stores an OpenGL vertex and fragment shader program.
 * 
 */
class Shader {
public:
    enum class MissingPolicy
    {
        Invalid = 0,
        Error,
        Print,
        Ignore,
        Count
    };

private:
    // Missing policy when a uniform or attribute cannot be found in the program
    static MissingPolicy missingPolicy;

    // Name of the shader, same as the key in the ShaderServer map
    std::string name;

    // ID of the vertex shader
    GLuint vShaderID;

    // ID of the fragment shader
    GLuint fShaderID;

    // ID of the linked program
    GLuint programID;

public:
    /**
     * @brief Construct a new Shader object
     * 
     * @param name string key used in the ShaderServer map
     * @param vertexShaderPath vertex shader file path, relative to the executable
     * @param fragmentShaderPath fragment shader file path, relative to the executable
     */
    Shader(const std::string& name, const std::string& vertexShaderPath, const std::string& fragmentShaderPath);
    ~Shader();

    Shader(const Shader&) = delete;
    Shader& operator=(const Shader&) = delete;
    Shader(Shader&&) = delete;
    Shader& operator=(Shader&&) = delete;

    /**
     * @brief Initializes the shader in OpenGL
     * 
     * @param vertexShaderPath vertex shader file path, relative to the executable
     * @param fragmentShaderPath fragment shader file path, relative to the executable
     */
    void init(const std::string& vertexShaderPath, const std::string& fragmentShaderPath);

    /**
     * @brief Destroys the OpenGL shader
     * 
     */
    void destroy();

    /**
     * @brief Get the Program ID
     * 
     * @return GLuint 
     */
    GLuint getProgramID();

    /**
     * @brief Get the program location of the uniform
     * 
     * @param uniformName name of the uniform found in the program source
     * @return GLuint 
     */
    GLuint getUniformLocation(const std::string& uniformName);

    /**
     * @brief Get the program location of the attribute
     * 
     * @param attributeName name of the attribute found in the program source
     * @return GLuint 
     */
    GLuint getAttributeLocation(const std::string& attributeName);

    /**
     * @brief Get the Name object
     * 
     * @return const std::string& 
     */
    const std::string& getName() const;

    /**
     * @brief Shorthand for glUseProgram(programID)
     * 
     */
    void bind();

    /**
     * @brief Shorthand for glUseProgram(0)
     * 
     */
    void unbind();

    static void setMissingPolicy(MissingPolicy policy) { missingPolicy = policy; }
};

// ------------------------------------------------------------
// Shader Server class
// ------------------------------------------------------------

/**
 * @brief Global class used to load and service shaders
 * 
 * Stores all shaders as unique_ptrs and will clean up at program end.
 * 
 */
class ShaderServer {
public:
    enum class DuplicatePolicy {
        Invalid = 0,
        Error,
        Print,
        Ignore,
        Replace,
        Count
    };

private:
    // Stores all shaders for the program
    static std::unordered_map<std::string, std::unique_ptr<Shader>> shaderMap;

    // The policy for trying to load a shader 
    static ShaderServer::DuplicatePolicy duplicatePolicy;

public:
    // There should be no instances of this class, everything is static 
    ShaderServer() = delete;
    ~ShaderServer() = delete;

    /**
     * @brief Load a shader into the server from source. 
     * 
     * Runs check if shaderName key already exists.
     * Creates a shader program using the source files through the shader constructor.
     * Adds the shader to the shader map. 
     * 
     * @param shaderName key for accessing the shader in the server map
     * @param vertexShaderPath vertex shader file path, relative to the executable
     * @param fragmentShaderPath fragment shader file path, relative to the executable
     */
    static void loadShader(const std::string& shaderName, const std::string& vertexShaderPath, const std::string& fragmentShaderPath);

    /**
     * @brief Get a shader pointer from the shader map.
     * 
     * @param shaderName key for accessing the shader in the shader map. Defined on load.
     * @return Shader* 
     */
    static Shader* getShader(const std::string& shaderName);

    /**
     * @brief Removes a shader from the shader map.
     * 
     * @param shaderName key for accessing the shader in the shader map. Defined on load.
     */
    static void removeShader(const std::string& shaderName);
    static void clear() { shaderMap.clear(); }
};