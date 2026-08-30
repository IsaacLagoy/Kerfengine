#include "Collider2D.h"
#include "ColliderPolygon2DMesh.h"

#include <stdexcept>


ColliderPolygon2D::ColliderPolygon2D(ColliderPolygon2DMesh* mesh) :
    scale(1.0f),
    mesh(mesh)
{
    if (!mesh) {
        throw std::runtime_error("ColliderPolygon2D mesh is null");
    }

    area = mesh->getArea();
}

void ColliderPolygon2D::setScale(const glm::vec2& scale)
{
    this->scale = scale;
    area = mesh->getArea() * glm::abs(scale.x * scale.y);
}

float ColliderPolygon2D::getInertia(float density) const
{
    return mesh->getInertia(density, scale);
}
