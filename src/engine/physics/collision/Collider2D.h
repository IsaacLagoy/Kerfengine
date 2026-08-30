#pragma once

#include <glm/glm.hpp>


// forward declarations
class ColliderPolygon2DMesh;

// ------------------------------------------------
// Collider2D class - Interface
// ------------------------------------------------

class Collider2D {
protected:
    float area;

public:
    virtual float getInertia(float density) const = 0;

    float getMass(float density) const { return density * area; };
    float getArea() const { return area; };
};

// ------------------------------------------------
// Collider2D class - Box
// ------------------------------------------------

class ColliderBox2D : public Collider2D {
private:
    float width;
    float height;

public:
    ColliderBox2D(float width, float height);
    float getInertia(float density) const override;
};

// ------------------------------------------------
// Collider2D class - Circle
// ------------------------------------------------

class ColliderCircle2D : public Collider2D {
private:
    float radius;

public:
    ColliderCircle2D(float radius);
    float getInertia(float density) const override;
};

// ------------------------------------------------
// Collider2D class - Polygon
// ------------------------------------------------

class ColliderPolygon2D : public Collider2D {
private:
    glm::vec2 scale;
    ColliderPolygon2DMesh* mesh;

public:
    ColliderPolygon2D(ColliderPolygon2DMesh* mesh);
    float getInertia(float density) const override;

    void setScale(const glm::vec2& scale);
    glm::vec2 getScale() const { return scale; }
    ColliderPolygon2DMesh* getMesh() const { return mesh; }
};