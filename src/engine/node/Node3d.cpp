#include <kerf/engine/node/Node3d.h>


namespace kerf {

// ------------------------------------------------------------
// Constructors
// ------------------------------------------------------------

Node3d::Node3d()
{
    this->position = glm::vec3(0.0f, 0.0f, 0.0f);
    this->rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
    this->scale = glm::vec3(1.0f, 1.0f, 1.0f);

    updateModelMatrix(position, rotation, scale);
}

Node3d::Node3d(const glm::vec3& position, const glm::quat& rotation, const glm::vec3& scale)
{
    this->position = position;
    this->rotation = rotation;
    this->scale = scale;

    updateModelMatrix(position, rotation, scale);
}

Node3d::~Node3d()
{
    // no op
}

// ------------------------------------------------------------
// Setters
// ------------------------------------------------------------

void Node3d::setPosition(const glm::vec3& position)
{
    this->position = position;
    updateModelMatrix(position, rotation, scale);
}

void Node3d::setRotation(const glm::quat& rotation)
{
    this->rotation = rotation;
    updateModelMatrix(position, rotation, scale);
}

void Node3d::setScale(const glm::vec3& scale)
{
    this->scale = scale;
    updateModelMatrix(position, rotation, scale);
}

// ------------------------------------------------------------
// Getters
// ------------------------------------------------------------

glm::vec3 Node3d::getPosition() const
{
    return position;
}

glm::quat Node3d::getRotation() const
{
    return rotation;
}

glm::vec3 Node3d::getScale() const
{
    return scale;
}

void Node3d::draw(const glm::mat4& viewProjection)
{
    Node::draw(viewProjection);
}

} // namespace kerf