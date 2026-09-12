#include "ColliderPolygon2DMesh.h"
#include "shared/Math.h"
#include "shared/Const.h"

#include <stdexcept>
#include <iostream>


ColliderPolygon2DMesh::ColliderPolygon2DMesh(const std::vector<glm::vec2>& vertices) : vertices(vertices)
{
    init();
}

ColliderPolygon2DMesh::ColliderPolygon2DMesh(const std::vector<glm::vec3>& vertices)
{
    this->vertices.reserve(vertices.size());
    for (const auto& vertex : vertices)
    {
        this->vertices.push_back(glm::vec2(vertex.x, vertex.y));
    }

    init();
}

float ColliderPolygon2DMesh::getInertia(float density, const glm::vec2& scale) const
{
    return polygonInertia(vertices, density, scale);
}

void ColliderPolygon2DMesh::init()
{
    bool success = monotoneChain(vertices, vertices);
    if (!success)
    {
        throw std::runtime_error("Failed to order vertices CCW");
    }

    faces.clear();
    const size_t n = vertices.size();
    if (n >= 3)
    {
        for (size_t i = 1; i + 1 < n; ++i)
        {
            faces.push_back(Face{
                vertices[0],
                vertices[i],
                vertices[i + 1]
            });
        }
    }

    area = polygonArea(vertices);
}

bool ColliderPolygon2DMesh::containsPoint(const glm::vec2& point) const
{
    for (const auto& face : faces)
    {
        if (faceContainsPoint(face, point))
        {
            return true;
        }
    }
    return false;
}

// ------------------------------------------------------------
// Initialize static variables
// ------------------------------------------------------------

std::unordered_map<std::string, std::unique_ptr<ColliderPolygon2DMesh>> ColliderPolygon2DMeshServer::meshMap;
ColliderPolygon2DMeshServer::DuplicatePolicy ColliderPolygon2DMeshServer::duplicatePolicy = ColliderPolygon2DMeshServer::DuplicatePolicy::Print;

void ColliderPolygon2DMeshServer::insertMesh(const std::string& meshName, std::unique_ptr<ColliderPolygon2DMesh> mesh)
{
    if (meshMap.find(meshName) == meshMap.end())
    {
        meshMap[meshName] = std::move(mesh);
        return;
    }

    switch (duplicatePolicy)
    {
        case DuplicatePolicy::Error:
            throw std::runtime_error(ANSI_RED + "[ColliderPolygon2DMeshServer] " + meshName + " mesh already exists!" + ANSI_RESET);

        case DuplicatePolicy::Print:
            std::cerr << ANSI_YELLOW << "[ColliderPolygon2DMeshServer] " + meshName + " mesh already exists!" << ANSI_RESET << std::endl;
            return;

        case DuplicatePolicy::Ignore:
            return;

        case DuplicatePolicy::Replace:
            meshMap[meshName] = std::move(mesh);
            return;

        default:
            throw std::runtime_error(ANSI_RED + "[ColliderPolygon2DMeshServer] invalid duplicate policy!" + ANSI_RESET);
    }
}

void ColliderPolygon2DMeshServer::loadMesh(const std::string& meshName, const std::vector<glm::vec2>& vertices)
{
    insertMesh(meshName, std::make_unique<ColliderPolygon2DMesh>(vertices));
}

void ColliderPolygon2DMeshServer::loadMesh(const std::string& meshName, const std::vector<glm::vec3>& vertices)
{
    insertMesh(meshName, std::make_unique<ColliderPolygon2DMesh>(vertices));
}

ColliderPolygon2DMesh* ColliderPolygon2DMeshServer::getMesh(const std::string& meshName)
{
    auto itr = meshMap.find(meshName);
    if (itr == meshMap.end())
    {
        throw std::runtime_error(ANSI_RED + "[ColliderPolygon2DMeshServer] " + meshName + " mesh not found!" + ANSI_RESET);
    }
    return itr->second.get();
}

void ColliderPolygon2DMeshServer::removeMesh(const std::string& meshName)
{
    auto itr = meshMap.find(meshName);
    if (itr == meshMap.end())
    {
        throw std::runtime_error(ANSI_RED + "[ColliderPolygon2DMeshServer] " + meshName + " mesh not found!" + ANSI_RESET);
    }
    meshMap.erase(itr);
}
