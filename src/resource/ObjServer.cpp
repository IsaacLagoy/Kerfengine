#include "ObjServer.h"

#include "shared/Const.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>
#include <vector>
#include <iostream>

// ------------------------------------------------------------
// Initialize static variables
// ------------------------------------------------------------

std::unordered_map<std::string, std::unique_ptr<Mesh>> ObjServer::meshMap;
ObjServer::DuplicatePolicy ObjServer::duplicatePolicy = ObjServer::DuplicatePolicy::Print;

// ------------------------------------
// Mesh
// ------------------------------------

Mesh::Mesh(const std::string& meshName, const std::string& objPath) : name(meshName)
{
    init(objPath);
}

Mesh::~Mesh()
{
    destroy();
}

void Mesh::init(const std::string& objPath)
{
    // set up tinyobj elements
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warnStr, errStr;

    // load file
    int rc = tinyobj::LoadObj(&attrib, &shapes, &materials, &warnStr, &errStr, objPath.c_str());
    if (!rc) 
    {
        throw std::runtime_error(ANSI_RED + "[ObjServer] " + objPath + " failed to load!" + errStr + ANSI_RESET);
    }

    // load mesh buffers
    std::vector<float> positions;
    std::vector<float> normals;
    std::vector<float> textureCoords;

    for(size_t s = 0; s < shapes.size(); s++) 
    {
        size_t index_offset = 0;
        for(size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) 
        {
            size_t fv = shapes[s].mesh.num_face_vertices[f];
            for(size_t v = 0; v < fv; v++) 
            {
                // vertices always need position
                tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];
                positions.push_back(attrib.vertices[3*idx.vertex_index+0]);
                positions.push_back(attrib.vertices[3*idx.vertex_index+1]);
                positions.push_back(attrib.vertices[3*idx.vertex_index+2]);

                // add normals if they exist
                if(!attrib.normals.empty()) 
                {
                    normals.push_back(attrib.normals[3*idx.normal_index+0]);
                    normals.push_back(attrib.normals[3*idx.normal_index+1]);
                    normals.push_back(attrib.normals[3*idx.normal_index+2]);
                }

                // add texture coords if they exist
                if (!attrib.texcoords.empty())
                {
                    textureCoords.push_back(attrib.texcoords[2*idx.texcoord_index+0]);
                    textureCoords.push_back(attrib.texcoords[2*idx.texcoord_index+1]);
                }
            }
            index_offset += fv;
        }
    }

    // Generate and bind VAO first
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    // Position — always attribute location 0
    glGenBuffers(1, &posBuf);
    glBindBuffer(GL_ARRAY_BUFFER, posBuf);
    glBufferData(GL_ARRAY_BUFFER, positions.size() * sizeof(float), positions.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*) 0);
 
    // Count vertices NOTE assumes triangle mesh
    vertCount = (int)(positions.size() / 3);

    // Normals — attribute location 1
    if (!normals.empty()) 
    {
        glGenBuffers(1, &norBuf);
        glBindBuffer(GL_ARRAY_BUFFER, norBuf);
        glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(float), normals.data(), GL_STATIC_DRAW);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*) 0);
        hasNor = true;
    }

    // Tex coords — attribute location 2
    if (!textureCoords.empty()) 
    {
        glGenBuffers(1, &texBuf);
        glBindBuffer(GL_ARRAY_BUFFER, texBuf);
        glBufferData(GL_ARRAY_BUFFER, textureCoords.size() * sizeof(float), textureCoords.data(), GL_STATIC_DRAW);
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, (void*) 0);
        hasTex  = true;
    }

    glBindVertexArray(0); // unbind — state is now saved inside the VAO
}

void Mesh::destroy()
{
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &posBuf);

    if (hasNor) 
    {
        glDeleteBuffers(1, &norBuf);
    }

    if (hasTex) 
    {
        glDeleteBuffers(1, &texBuf);
    }
}

void Mesh::draw() const
{
    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLES, 0, vertCount);
    glBindVertexArray(0);
}

// ------------------------------------
// ObjServer
// ------------------------------------

void ObjServer::loadMesh(const std::string& meshName, const std::string& objPath)
{
    // if mesh doesn't exist, simply build it
    if (meshMap.find(meshName) == meshMap.end()) 
    {
        meshMap[meshName] = std::make_unique<Mesh>(meshName, objPath);
        return;
    }

    // handle the case when the mesh does exist
    switch (duplicatePolicy)
    {
        case ObjServer::DuplicatePolicy::Error:
            throw std::runtime_error(ANSI_RED + "[ObjServer] " + meshName + " mesh already exists!" + ANSI_RESET);

        case ObjServer::DuplicatePolicy::Print:
            std::cerr << ANSI_YELLOW << "[ObjServer] " + meshName + " mesh already exists!" << ANSI_RESET << std::endl;
            return;

        case ObjServer::DuplicatePolicy::Ignore:
            return;

        // rebuild the mesh with the new vertex and fragment
        case ObjServer::DuplicatePolicy::Replace:
        {
            auto replacement = std::make_unique<Mesh>(meshName, objPath);
            meshMap[meshName] = std::move(replacement);
            return;
        }

        // should never reach
        default: 
            throw std::runtime_error(ANSI_RED + "[ObjServer] invalid duplicate policy!" + ANSI_RESET);
    }
}

Mesh* ObjServer::getMesh(const std::string& meshName)
{
    auto itr = meshMap.find(meshName);
    if (itr == meshMap.end())
    {
        throw std::runtime_error(ANSI_RED + "[ObjServer] " + meshName + " mesh not found!" + ANSI_RESET);
    }
    return itr->second.get();
}

void ObjServer::removeMesh(const std::string& meshName)
{
    auto itr = meshMap.find(meshName);
    if (itr == meshMap.end())
    {
        throw std::runtime_error(ANSI_RED + "[ObjServer] " + meshName + " mesh not found!" + ANSI_RESET);
    }
    meshMap.erase(itr);
}