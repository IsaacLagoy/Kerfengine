#pragma once

#include <glad/glad.h>
#include <string>
#include <vector>
#include <glm/glm.hpp>

namespace kerf {

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

        // location of the mesh's barycentric buffer
        GLuint baryBuf = 0;
    
        // number of vertices in the mesh
        int vertCount;
    
        // does the mesh have normals
        bool hasNor = false;
    
        // does the mesh have texture coords
        bool hasTex = false;
    
        // string key of the mesh as found in the ObjServer map
        std::string name;

        // vertices of the mesh
        std::vector<glm::vec3> vertices;
    
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

        const std::vector<glm::vec3>& getVertices() const;
    };

} // namespace kerf