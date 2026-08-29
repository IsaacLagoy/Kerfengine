#pragma once

#include <glad/glad.h>
#include <unordered_map>
#include <memory>
#include <string>


// -----------------------------------------------------
// Mesh Class
// -----------------------------------------------------

/**
 * @brief OpenGL mesh object
 * 
 * Does not use indices, instead it stores vertices directly into triplet buffers.
 * A potential performance incease could be adding indexing.
 * Supports both normal and texture coordinates.
 * Only supports triangle meshes.
 * 
 */
class Mesh {    
private:
    // location of the mesh's Vertex Array Object
    GLuint vao = 0;

    // location of the mesh's position buffer
    GLuint posBuf = 0;

    // location of the mesh's normal buffer
    GLuint norBuf = 0;

    // location of the mesh's texture buffer
    GLuint texBuf = 0;

    // number of vertices in the mesh
    int vertCount;

    // does the mesh have normals
    bool hasNor = false;

    // does the mesh have texture coords
    bool hasTex = false;

    // string key of the mesh as found in the ObjServer map
    std::string name;

public:
    /**
     * @brief Construct a new Mesh object
     * 
     * @param meshName 
     * @param objPath 
     */
    Mesh(const std::string& meshName, const std::string& objPath);
    ~Mesh();

    Mesh(const Mesh&) = delete;
    Mesh& operator=(const Mesh&) = delete;
    Mesh(Mesh&&) = delete;
    Mesh& operator=(Mesh&&) = delete;

    /**
     * @brief Initializes the map and uploads to OpenGL
     * 
     * @param objPath 
     */
    void init(const std::string& objPath);

    /**
     * @brief Destroys the OpenGL buffers associated with the mesh
     * 
     */
    void destroy();

    /**
     * @brief Draws the mesh with the VAO
     * 
     */
    void draw() const;
};

// -----------------------------------------------------
// ObjServer Class
// -----------------------------------------------------

/**
 * @brief 
 * 
 */
class ObjServer {
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
    // stores all meshes for the program 
    static std::unordered_map<std::string, std::unique_ptr<Mesh>> meshMap;

    // determines how the mesh handles duplicate mesh name keys
    static ObjServer::DuplicatePolicy duplicatePolicy;

public:
    // static only class, do not create an instances
    ObjServer() = delete;
    ~ObjServer() = delete;

    /**
     * @brief 
     * 
     * @param meshName 
     * @param objPath 
     */
    static void loadMesh(const std::string& meshName, const std::string& objPath);

    /**
     * @brief Get the Mesh object
     * 
     * @param meshName 
     * @return Mesh* 
     */
    static Mesh* getMesh(const std::string& meshName);

    /**
     * @brief Removes a mesh from the mesh map
     * 
     * @param meshName 
     */
    static void removeMesh(const std::string& meshName);
    static void clear() { meshMap.clear(); }
};