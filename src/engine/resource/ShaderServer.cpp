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

Shader::Shader(
    const std::string& name,
    const char* vertexSrc,
    const char* fragmentSrc,
    const char* vertexLabel,
    const char* fragmentLabel
) : name(name)
{
    compile(vertexSrc, fragmentSrc, vertexLabel, fragmentLabel);
}

Shader::~Shader()
{
    destroy();
}

void Shader::compile(
    const char* vertexSrc,
    const char* fragmentSrc,
    const char* vertexLabel,
    const char* fragmentLabel
)
{
    this->vShaderID = glCreateShader(GL_VERTEX_SHADER);
    this->fShaderID = glCreateShader(GL_FRAGMENT_SHADER);

    glShaderSource(vShaderID, 1, &vertexSrc, nullptr);
    glShaderSource(fShaderID, 1, &fragmentSrc, nullptr);

    int rc = 0;
    glCompileShader(vShaderID);
    glGetShaderiv(vShaderID, GL_COMPILE_STATUS, &rc);
    if (rc == GL_FALSE)
    {
        char log[512];
        glGetShaderInfoLog(vShaderID, sizeof(log), nullptr, log);
        throw std::runtime_error(ANSI_RED + "[Shader] vertex shader compilation failed (" + vertexLabel + "):\n" + ANSI_RESET + log);
    }

    glCompileShader(fShaderID);
    glGetShaderiv(fShaderID, GL_COMPILE_STATUS, &rc);
    if (rc == GL_FALSE)
    {
        char log[512];
        glGetShaderInfoLog(fShaderID, sizeof(log), nullptr, log);
        throw std::runtime_error(ANSI_RED + "[Shader] fragment shader compilation failed (" + fragmentLabel + "):\n" + ANSI_RESET + log);
    }

    this->programID = glCreateProgram();
    glAttachShader(programID, vShaderID);
    glAttachShader(programID, fShaderID);
    glLinkProgram(programID);
    glGetProgramiv(programID, GL_LINK_STATUS, &rc);
    if (rc == GL_FALSE)
    {
        char log[512];
        glGetProgramInfoLog(programID, sizeof(log), nullptr, log);
        throw std::runtime_error(ANSI_RED + "[Shader] linking failed (" + vertexLabel + " + " + fragmentLabel + "):\n" + ANSI_RESET + log);
    }
}

void Shader::init(const std::string& vertexShaderPath, const std::string& fragmentShaderPath)
{
    const std::string vShaderSrc = Files::readTextFile(Files::resolvePath(vertexShaderPath));
    const std::string fShaderSrc = Files::readTextFile(Files::resolvePath(fragmentShaderPath));
    compile(vShaderSrc.c_str(), fShaderSrc.c_str(), vertexShaderPath.c_str(), fragmentShaderPath.c_str());
}

void Shader::destroy()
{
    if (vShaderID)
    {
        glDeleteShader(vShaderID);
        vShaderID = 0;
    }
    if (fShaderID)
    {
        glDeleteShader(fShaderID);
        fShaderID = 0;
    }
    if (programID)
    {
        glDeleteProgram(programID);
        programID = 0;
    }
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

void ShaderServer::loadShaderFromSource(
    const std::string& shaderName,
    const char* vertexSrc,
    const char* fragmentSrc
)
{
    const std::string vertexLabel = shaderName + ".vert";
    const std::string fragmentLabel = shaderName + ".frag";

    if (shaderMap.find(shaderName) == shaderMap.end())
    {
        shaderMap[shaderName] = std::make_unique<Shader>(
            shaderName, vertexSrc, fragmentSrc, vertexLabel.c_str(), fragmentLabel.c_str()
        );
        return;
    }

    switch (duplicatePolicy)
    {
        case ShaderServer::DuplicatePolicy::Error:
            throw std::runtime_error(ANSI_RED + "[ShaderServer] " + shaderName + " shader already exists!" + ANSI_RESET);

        case ShaderServer::DuplicatePolicy::Print:
            std::cerr << ANSI_YELLOW << "[ShaderServer] " + shaderName + " shader already exists!" << ANSI_RESET << std::endl;
            return;

        case ShaderServer::DuplicatePolicy::Ignore:
            return;

        case ShaderServer::DuplicatePolicy::Replace:
            shaderMap[shaderName] = std::make_unique<Shader>(
                shaderName, vertexSrc, fragmentSrc, vertexLabel.c_str(), fragmentLabel.c_str()
            );
            return;

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