#include "engine/node/RigidBody.h"


RigidBody::RigidBody() : Model() {}

RigidBody::RigidBody(const glm::vec3& pose, const glm::vec2& scale, Mesh* mesh, Material* material, const Collider2D& collider, float density, const glm::vec3& velocity) :
    Model(pose, scale, mesh, material),
    density(density),
    velocity(velocity),
    collider(collider)
{
    initMass();
}

RigidBody::~RigidBody()
{
    unlinkRigidBody();
}

void RigidBody::initMass()
{
    if (collider.type == ColliderType::INVALID) {
        mass = -1.0f;
        return;
    }

    mass = collider.getMass(density, getScale());
}

void RigidBody::update(float dt)
{
    setPose(getPose() + velocity * dt);
}

void RigidBody::insertRigidBody(RigidBody* pos)
{
    this->nextRigidBody = pos;
    this->prevRigidBody = pos->prevRigidBody;
    pos->prevRigidBody->nextRigidBody = this;
    pos->prevRigidBody = this;
}

void RigidBody::unlinkRigidBody()
{
    if (prevRigidBody) {
        prevRigidBody->nextRigidBody = nextRigidBody;
    }

    if (nextRigidBody) {
        nextRigidBody->prevRigidBody = prevRigidBody;
    }

    nextRigidBody = nullptr;
    prevRigidBody = nullptr;
}

void RigidBody::draw(const glm::mat4& viewProjection)
{
    Model::draw(viewProjection);
}