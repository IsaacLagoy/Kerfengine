#include "Collider2D.h"

#include <glm/gtc/constants.hpp>

ColliderCircle2D::ColliderCircle2D(float radius) : radius(radius)
{
    area = glm::pi<float>() * radius * radius;
}

float ColliderCircle2D::getInertia(float density) const
{
    return (1.0f / 2.0f) * getMass(density) * radius * radius;
}