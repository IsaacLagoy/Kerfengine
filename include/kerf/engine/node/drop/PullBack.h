#pragma once

#include <glm/glm.hpp>


namespace kerf {

/**
 * @brief Per-drag "lift, then pull back to the slot" state used by sticky drop targets.
 *
 * Pure value type: it knows nothing about Drag, locks, or the scene. The owner decides
 * when the drag is lifted or seated, feeds in positions, and applies the result
 * (pose and lock) to the drag itself. This lets a target that holds many drags keep
 * one PullBack per drag, so several can return at once.
 *
 * States:
 *  - Seated:    resting in its slot, nothing to do.
 *  - Lifted:    taken out of the slot (in hand, or released somewhere that did not claim it).
 *  - Returning: easing from where it was released back to the slot.
 *
 * The return target is read on every advance() call, so the slot may move while the
 * drag is on its way back.
 */
class PullBack {
public:
    enum class State { Seated, Lifted, Returning };

private:
    State state = State::Seated;
    float duration = 1.0f; // always > 0, see setDuration
    float elapsed = 0.0f;
    glm::vec2 from = glm::vec2(0.0f);

public:
    explicit PullBack(float duration = 1.0f);

    // getters
    State getState() const;
    bool isSeated() const;
    bool isLifted() const;     // Lifted or Returning, i.e. out of the slot
    bool isReturning() const;
    float getDuration() const;

    // setters
    // clamped to a small positive minimum so progress never divides by zero
    void setDuration(float duration);

    // The drag left the slot. Cancels any return in progress.
    void lift();

    // The drag is back in the slot (or no longer tracked). Cancels any return in progress.
    void seat();

    // Start returning from `position` if lifted and not already returning.
    // Returns true if a return was started by this call.
    bool beginReturn(const glm::vec2& position);

    // Advance a return by dt toward the live target and return the drag's new position.
    // On the call that finishes, the state becomes Seated and the target is returned exactly.
    // Only meaningful while returning; otherwise returns target unchanged.
    glm::vec2 advance(float dt, const glm::vec2& target);
};

} // namespace kerf
