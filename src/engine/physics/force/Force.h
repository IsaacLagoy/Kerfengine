#pragma once

#include "engine/physics/Const.h"

#include <glm/glm.hpp>


namespace kerf {
    
// forward declarations
class RigidBody;


class Force {
private:
    RigidBody* bodyA;
    RigidBody* bodyB;
    Force* nextA;
    Force* nextB;
    Force* next;
    Force* prev;
    Force* prevA;
    Force* prevB;

    glm::vec3 J[MAX_ROWS];
    glm::mat3 H[MAX_ROWS];
    float C[MAX_ROWS];
    float fmin[MAX_ROWS];
    float fmax[MAX_ROWS];
    float stiffness[MAX_ROWS];
    float fracture[MAX_ROWS];
    float penalty[MAX_ROWS];
    float lambda[MAX_ROWS];

public:
    Force(RigidBody* bodyA, RigidBody* bodyB);
    virtual ~Force();

    void diable();

    virtual int rows() const = 0;
    virtual bool init() = 0;
    virtual void computeConstraint(float alpha) = 0;
    virtual void computeDerivatives(RigidBody* body) = 0;
};

} // namespace kerf