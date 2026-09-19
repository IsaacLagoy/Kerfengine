#include <kerf/engine/node/Node2d.h>


namespace kerf {
    
// ------------------------------------------------------------
// Constructors
// ------------------------------------------------------------

Node2d::Node2d()
{
    this->pose = glm::vec3(0.0f);
    this->scale = glm::vec2(1.0f);

    updateModelMatrix(pose, scale);
}

Node2d::Node2d(const glm::vec3& pose)
{
    this->pose = pose;
    this->scale = glm::vec2(1.0f);

    updateModelMatrix(pose, scale);
}

Node2d::Node2d(const glm::vec3& pose, const glm::vec2& scale)
{
    this->pose = pose;
    this->scale = scale;

    updateModelMatrix(pose, scale);
}

Node2d::~Node2d()
{
    // no op
}

// ------------------------------------------------------------
// Setters
// ------------------------------------------------------------

void Node2d::setPose(const glm::vec3& pose)
{
    this->pose = pose;
    updateModelMatrix(pose, scale);
}

void Node2d::setScale(const glm::vec2& scale)
{
    this->scale = scale;
    updateModelMatrix(pose, scale);
}

// ------------------------------------------------------------
// Getters
// ------------------------------------------------------------

glm::vec3 Node2d::getPose() const
{
    return pose;
}

glm::vec2 Node2d::getScale() const
{
    return scale;
}

void Node2d::draw(const glm::mat4& viewProjection)
{
    Node::draw(viewProjection);
}

} // namespace kerf