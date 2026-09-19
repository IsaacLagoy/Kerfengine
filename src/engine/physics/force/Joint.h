#pragma once

#include "engine/physics/force/Force.h"
#include "shared/Const.h"

namespace kerf {

class Joint : public Force {
private:
    glm::vec2 rA, rB;
    glm::vec3 C0;
    float torqueArm;
    float restAngle;



public:
    Joint(RigidBody* bodyA, RigidBody* bodyB, glm::vec2 rA, glm::vec2 rB, glm::vec3 stiffness = glm::vec3(INFINITY_FLOAT), float fracture = INFINITY_FLOAT);

    int rows() const override { return 3; }
    bool init() override;
    void computeConstraint(float alpha) override;
    void computeDerivatives(RigidBody* body) override;

    void setTorqueArm(float torqueArm);
    void setRestAngle(float restAngle);
};

} // namespace kerf