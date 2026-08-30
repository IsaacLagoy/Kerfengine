#include "Collider2D.h"

ColliderBox2D::ColliderBox2D(float width, float height) :
    width(width), 
    height(height)
{
    area = width * height;
}

float ColliderBox2D::getInertia(float density) const
{
    return (1.0f / 12.0f) * getMass(density) * (width * width + height * height);
}