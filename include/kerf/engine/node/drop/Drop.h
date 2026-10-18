#pragma once

#include <kerf/engine/node/drop/DropBase.h>


namespace kerf {

// forward declarations
class Drag;

/**
 * @brief A drop target that holds at most one drag.
 *
 * Locking, callbacks, and the claim flow live in DropBase.
 */
class Drop : public DropBase {
private:
    Drag* drag = nullptr;

public:
    Drop(const glm::vec3& pose, const glm::vec2& scale, Mesh* mesh, Material* material, Shader* shader, const Collider2D& collider);
    ~Drop();

    virtual void updateButton(float dt, const glm::vec2& mousePosition, bool mouseDown, bool mousePressed) override;

    // getters
    Drag* getDrag() const;

    // setters
    virtual void setDrag(Drag* drag);

protected:
    bool canAccept(const Drag* incoming) const override;
    void accept(Drag* incoming) override;
    void forgetDrag(Drag* leaving) override;

    void assignParkedDrop(Drag* drag);
};

} // namespace kerf
