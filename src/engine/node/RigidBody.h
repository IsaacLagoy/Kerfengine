#pragma once

#include "engine/node/Model.h"
#include "engine/physics/collision/Collider2D.h"

// forward declarations
class Scene;
class Shader;

class RigidBody : public Model {
    friend class Scene;

private:
    float density = -1.0f;
    float mass = -1.0f;
    glm::vec3 velocity = glm::vec3(0.0f);
    Collider2D collider;

    RigidBody* nextRigidBody = nullptr;
    RigidBody* prevRigidBody = nullptr;

public:
    RigidBody(const glm::vec3& pose, const glm::vec2& scale, Mesh* mesh, Material* material, Shader* shader, const Collider2D& collider, float density, const glm::vec3& velocity);
    ~RigidBody();

    void update(float dt);

    const Collider2D& getCollider() const { return collider; }

protected:
    // protected constructor for scene sentinel nodes
    RigidBody();

    void insertRigidBody(RigidBody* pos);
    void unlinkRigidBody();
    virtual void draw(const glm::mat4& viewProjection) override;

private:
    void initMass();
};
