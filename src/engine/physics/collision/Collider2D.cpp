#include "engine/physics/collision/Collider2D.h"
#include "engine/physics/collision/ColliderPolygon2DMesh.h"

#include <stdexcept>
#include <glm/gtc/constants.hpp>


Collider2D Collider2D::box(float width, float height)
{
    Collider2D collider;
    collider.type = ColliderType::BOX;
    collider.area = width * height;
    collider.boxShape.width = width;
    collider.boxShape.height = height;
    return collider;
}

Collider2D Collider2D::circle(float radius)
{
    Collider2D collider;
    collider.type = ColliderType::CIRCLE;
    collider.area = glm::pi<float>() * radius * radius;
    collider.radius = radius;
    return collider;
}

Collider2D Collider2D::polygon(ColliderPolygon2DMesh* mesh)
{
    if (!mesh) {
        throw std::runtime_error("Collider2D polygon mesh is null");
    }

    Collider2D collider;
    collider.type = ColliderType::POLYGON;
    collider.area = mesh->getArea();
    collider.mesh = mesh;
    return collider;
}

float Collider2D::getArea(const glm::vec2& scale) const
{
    return area * glm::abs(scale.x * scale.y);
}

float Collider2D::getMass(float density, const glm::vec2& scale) const
{
    return density * getArea(scale);
}

float Collider2D::getInertia(float density, const glm::vec2& scale) const
{
    const float mass = getMass(density, scale);

    switch (type) {
        case ColliderType::BOX: {
            const float w = boxShape.width * scale.x;
            const float h = boxShape.height * scale.y;
            return (1.0f / 12.0f) * mass * (w * w + h * h);
        }
        case ColliderType::CIRCLE: {
            const float a = radius * scale.x;
            const float b = radius * scale.y;
            return (1.0f / 4.0f) * mass * (a * a + b * b);
        }
        case ColliderType::POLYGON: {
            if (!mesh) {
                return 0.0f;
            }
            return mesh->getInertia(density, scale);
        }
        default:
            return 0.0f;
    }
}

const ColliderPolygon2DMesh* Collider2D::getMesh() const
{
    assert(type == ColliderType::POLYGON);
    return mesh;
}