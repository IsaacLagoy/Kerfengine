#pragma once

// forward declarations
class Force;
class RigidBody;

class Solver {
private:
    int iterations;

    float alpha; // stabilization
    float beta; // penalty ramping
    float gamma; // warmstarting decay parameter

    bool postStabilize;

    RigidBody* bodies;
    Force* forces;

public:
    Solver();
    ~Solver();

    void step(float dt);
};