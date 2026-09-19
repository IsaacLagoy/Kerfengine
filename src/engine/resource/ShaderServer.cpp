#include <kerf/engine/resource/ShaderServer.h>
#include <stdexcept>
#include <iostream>

#include "shared/Const.h"
#include "shared/Files.h"


namespace kerf {

// ------------------------------------------------------------
// Initialize static variables
// ------------------------------------------------------------

ShaderServer::DuplicatePolicy ShaderServer::duplicatePolicy = ShaderServer::DuplicatePolicy::Print;
std::unordered_map<std::string, std::unique_ptr<Shader>> ShaderServer::shaderMap;

// ------------------------------------------------------------
// Shader class
// ------------------------------------------------------------

Shader::Shader(const std::string& name, const std::string& vertexShaderPath, const std::string& fragmentShaderPath) : name(name)
{
    init(vertexShaderPath, fragmentShaderPath);
}

Shader::~Shader()
{
    destroy();
}

void Shader::init(const std::string& vertexShaderPath, const std::string& fragmentShaderPath)
{
    // create shader handles
    this->vShaderID = glCreateShader(GL_VERTEX_SHADER);
    this->fShaderID = glCreateShader(GL_FRAGMENT_SHADER);

    // read shader source
    std::string vShaderSrc = Files::readTextFile(Files::resolvePath(vertexShaderPath));
    std::string fShaderSrc = Files::readTextFile(Files::resolvePath(fragmentShaderPath));
    const char* vShaderText = vShaderSrc.c_str();
    const char* fShaderText = fShaderSrc.c_str();
    glShaderSource(vShaderID, 1, &vShaderText, NULL);
    glShaderSource(fShaderID, 1, &fShaderText, NULL);

    // compile vertex shader
    int rc;
    glCompileShader(vShaderID);
    glGetShaderiv(vShaderID, GL_COMPILE_STATUS, &rc);

    // failed to cmopile vertex shader
    if (rc == GL_FALSE) 
    {
        char log[512];
        glGetShaderInfoLog(vShaderID, sizeof(log), nullptr, log);
        throw std::runtime_error(ANSI_RED + "[Shader] vertex shader compilation failed (" + vertexShaderPath + "):\n" + ANSI_RESET + log);
    }

    // compile fragment shader
    glCompileShader(fShaderID);
    glGetShaderiv(fShaderID, GL_COMPILE_STATUS, &rc);

    // failed to compile fragment shader
    if (rc == GL_FALSE) 
    {
        char log[512];
        glGetShaderInfoLog(fShaderID, sizeof(log), nullptr, log);
        throw std::runtime_error(ANSI_RED + "[Shader] fragment shader compilation failed (" + fragmentShaderPath + "):\n" + ANSI_RESET + log);
    }

    // link shader
    this->programID = glCreateProgram();
    glAttachShader(programID, vShaderID);
    glAttachShader(programID, fShaderID);
    glLinkProgram(programID);
    glGetProgramiv(programID, GL_LINK_STATUS, &rc);

    // failed to link shaders
    if (rc == GL_FALSE) 
    {
        char log[512];
        glGetProgramInfoLog(programID, sizeof(log), nullptr, log);
        throw std::runtime_error(ANSI_RED + "[Shader] linking failed (" + vertexShaderPath + " + " + fragmentShaderPath + "):\n" + ANSI_RESET + log);
    }
}

void Shader::destroy()
{
    glDeleteShader(vShaderID);
    glDeleteShader(fShaderID);
    glDeleteProgram(programID);
}

GLuint Shader::getProgramID()
{
    return programID;
}

GLuint Shader::getUniformLocation(const std::string& name)
{
    return glGetUniformLocation(programID, name.c_str());
}

GLuint Shader::getAttributeLocation(const std::string& name)
{
    return glGetAttribLocation(programID, name.c_str());
}

const std::string& Shader::getName() const
{
    return name;
}

void Shader::bind()
{
    glUseProgram(programID);
}

void Shader::unbind()
{
    glUseProgram(0);
}

// ------------------------------------------------------------
// ShaderServer class
// ------------------------------------------------------------

void ShaderServer::loadShader(const std::string& shaderName, const std::string& vertexShaderPath, const std::string& fragmentShaderPath)
{
    // if shader doesn't exist, simply build it
    if (shaderMap.find(shaderName) == shaderMap.end()) 
    {
        shaderMap[shaderName] = std::make_unique<Shader>(shaderName, vertexShaderPath, fragmentShaderPath);
        return;
    }

    // handle the case when the shader does exist
    switch (duplicatePolicy)
    {
        case ShaderServer::DuplicatePolicy::Error:
            throw std::runtime_error(ANSI_RED + "[ShaderServer] " + shaderName + " shader already exists!" + ANSI_RESET);

        case ShaderServer::DuplicatePolicy::Print:
            std::cerr << ANSI_YELLOW << "[ShaderServer] " + shaderName + " shader already exists!" << ANSI_RESET << std::endl;
            return;

        case ShaderServer::DuplicatePolicy::Ignore:
            return;

        // rebuild the shader with the new vertex and fragment
        case ShaderServer::DuplicatePolicy::Replace:
        {
            auto replacement = std::make_unique<Shader>(shaderName, vertexShaderPath, fragmentShaderPath);
            shaderMap[shaderName] = std::move(replacement);
            return;
        }

        // should never reach
        default: 
            throw std::runtime_error(ANSI_RED + "[ShaderServer] invalid duplicate policy!" + ANSI_RESET);
    }
}

Shader* ShaderServer::getShader(const std::string& shaderName) 
{
    auto itr = shaderMap.find(shaderName);
    if (itr == shaderMap.end()) 
    {
        throw std::runtime_error(ANSI_RED + "[ShaderServer] " + shaderName + " shader not found!" + ANSI_RESET);
    }
    return itr->second.get();
}

void ShaderServer::removeShader(const std::string& shaderName)
{
    auto itr = shaderMap.find(shaderName);
    if (itr == shaderMap.end()) 
    {
        throw std::runtime_error(ANSI_RED + "[ShaderServer] " + shaderName + " shader not found!" + ANSI_RESET);
    }
    shaderMap.erase(itr);
}

} // namespace kerf