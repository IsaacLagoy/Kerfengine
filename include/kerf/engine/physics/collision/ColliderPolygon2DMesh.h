#pragma once

#include <vector>
#include <unordered_map>
#include <memory>
#include <string>
#include <glm/glm.hpp>

#include <kerf/shared/Structs.h>


namespace kerf {

// ------------------------------------------------
// ColliderPolygon2DMesh
// ------------------------------------------------

class ColliderPolygon2DMesh {
private:
    std::vector<glm::vec2> vertices;
    std::vector<Face> faces;
    float area = 0.0f;

public:
    ColliderPolygon2DMesh(const std::vector<glm::vec2>& vertices);
    ColliderPolygon2DMesh(const std::vector<glm::vec3>& vertices);

    float getInertia(float density, const glm::vec2& scale) const;
    float getArea() const { return area; }
    const std::vector<glm::vec2>& getVertices() const { return vertices; }
    const std::vector<Face>& getFaces() const { return faces; }

    bool containsPoint(const glm::vec2& point) const;

private:
    void init();
};

// ------------------------------------------------
// ColliderPolygon2DMeshServer
// ------------------------------------------------

class ColliderPolygon2DMeshServer {
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
    static std::unordered_map<std::string, std::unique_ptr<ColliderPolygon2DMesh>> meshMap;
    static DuplicatePolicy duplicatePolicy;

public:
    ColliderPolygon2DMeshServer() = delete;
    ~ColliderPolygon2DMeshServer() = delete;

    static void loadMesh(const std::string& meshName, const std::vector<glm::vec2>& vertices);
    static void loadMesh(const std::string& meshName, const std::vector<glm::vec3>& vertices);
    static ColliderPolygon2DMesh* getMesh(const std::string& meshName);
    static void removeMesh(const std::string& meshName);
    static void clear() { meshMap.clear(); }
    static void setDuplicatePolicy(DuplicatePolicy policy) { duplicatePolicy = policy; }

private:
    static void insertMesh(const std::string& meshName, std::unique_ptr<ColliderPolygon2DMesh> mesh);
};

} // namespace kerf