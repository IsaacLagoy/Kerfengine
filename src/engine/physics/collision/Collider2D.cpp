#include <kerf/engine/physics/collision/Collider2D.h>
#include <kerf/engine/physics/collision/ColliderPolygon2DMesh.h>
#include <stdexcept>
#include <glm/gtc/constants.hpp>


namespace kerf {
    
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

bool Collider2D::isPointInside(const glm::vec2& point) const
{
    switch (type)
    {
        case ColliderType::POLYGON:
            return isPointInsidePolygon(point);
        case ColliderType::CIRCLE:
            return isPointInsideCircle(point);
        case ColliderType::BOX:
            return isPointInsideBox(point);
        default:
            throw std::runtime_error("Collider2D isPointInside: invalid collider type");
    }
}

bool Collider2D::isPointInsidePolygon(const glm::vec2& point) const
{
    return mesh->containsPoint(point);
}

bool Collider2D::isPointInsideCircle(const glm::vec2& point) const
{
    return glm::length(point) <= radius;
}

bool Collider2D::isPointInsideBox(const glm::vec2& point) const
{
    return point.x >= -boxShape.width / 2.0f && point.x <= boxShape.width / 2.0f &&
           point.y >= -boxShape.height / 2.0f && point.y <= boxShape.height / 2.0f;
}

} // namespace kerf