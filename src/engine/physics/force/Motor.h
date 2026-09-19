#pragma once

#include "engine/physics/force/Force.h"

namespace kerf {

class Motor : public Force {
private:
    float speed;

public:
    Motor(RigidBody* bodyA, RigidBody* bodyB, float speed, float maxTorque);

    int rows() const override { return 1; }

    bool init() override;
    void computeConstraint(float alpha) override;
    void computeDerivatives(RigidBody* body) override;
};

} // namespace kerf