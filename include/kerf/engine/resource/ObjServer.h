#pragma once

#include <kerf/engine/resource/Mesh.h>
#include <unordered_map>
#include <memory>
#include <string>

namespace kerf {

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
    static void loadMeshFromSource(const std::string& meshName, const char* objSrc);

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

} // namespace kerf