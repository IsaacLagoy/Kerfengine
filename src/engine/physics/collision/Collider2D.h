#pragma once

#include <glm/glm.hpp>


// forward declarations
class ColliderPolygon2DMesh;

enum class ColliderType {
    INVALID,
    BOX,
    CIRCLE,
    POLYGON
};

// ------------------------------------------------
// Collider2D - local-space shape data
// ------------------------------------------------

class Collider2D {
public:
    struct Box {
        float width;
        float height;
    };

    ColliderType type = ColliderType::INVALID;
    float area = 0.0f;

    union {
        Box boxShape;
        float radius;
        ColliderPolygon2DMesh* mesh;
    };

    static Collider2D box(float width, float height);
    static Collider2D circle(float radius);
    static Collider2D polygon(ColliderPolygon2DMesh* mesh);

    float getArea(const glm::vec2& scale = glm::vec2(1.0f)) const;
    float getMass(float density, const glm::vec2& scale = glm::vec2(1.0f)) const;
    float getInertia(float density, const glm::vec2& scale = glm::vec2(1.0f)) const;
};
