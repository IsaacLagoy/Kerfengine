#include <kerf/engine/resource/ObjServer.h>
#include <kerf/engine/resource/Mesh.h>
#include <memory>
#include <iostream>
#include <unordered_map>

#include "shared/Const.h"


namespace kerf {

// ------------------------------------------------------------
// Initialize static variables
// ------------------------------------------------------------

std::unordered_map<std::string, std::unique_ptr<Mesh>> ObjServer::meshMap;
ObjServer::DuplicatePolicy ObjServer::duplicatePolicy = ObjServer::DuplicatePolicy::Print;

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

} // namespace kerf