#pragma once

#include "engine/physics/force/Force.h"

namespace kerf {

class Spring : public Force {
private:
    glm::vec3 rA, rB;
    float rest;

public:
    Spring(RigidBody* bodyA, RigidBody* bodyB, glm::vec3 rA, glm::vec3 rB, float stiffness, float rest=-1.0f);

    int rows() const override { return 1; }

    bool init() override;
    void computeConstraint(float alpha) override;
    void computeDerivatives(RigidBody* body) override;
};

} // namespace kerf