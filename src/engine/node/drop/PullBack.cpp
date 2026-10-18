#include <kerf/engine/node/drop/PullBack.h>


namespace kerf {

namespace {

constexpr float kMinDuration = 1.0e-4f;

float sanitizeDuration(float duration)
{
    return duration < kMinDuration ? kMinDuration : duration;
}

float smoothstep(float t)
{
    return t * t * (3.0f - 2.0f * t);
}

} // namespace

PullBack::PullBack(float duration) :
    duration(sanitizeDuration(duration))
{}

PullBack::State PullBack::getState() const
{
    return state;
}

bool PullBack::isSeated() const
{
    return state == State::Seated;
}

bool PullBack::isLifted() const
{
    return state != State::Seated;
}

bool PullBack::isReturning() const
{
    return state == State::Returning;
}

float PullBack::getDuration() const
{
    return duration;
}

void PullBack::setDuration(float duration)
{
    this->duration = sanitizeDuration(duration);
}

void PullBack::lift()
{
    state = State::Lifted;
}

void PullBack::seat()
{
    state = State::Seated;
}

bool PullBack::beginReturn(const glm::vec2& position)
{
    if (state != State::Lifted)
    {
        return false;
    }

    state = State::Returning;
    elapsed = 0.0f;
    from = position;
    return true;
}

glm::vec2 PullBack::advance(float dt, const glm::vec2& target)
{
    if (state != State::Returning)
    {
        return target;
    }

    elapsed += dt;
    if (elapsed >= duration)
    {
        state = State::Seated;
        return target;
    }

    const float t = smoothstep(glm::clamp(elapsed / duration, 0.0f, 1.0f));
    return glm::mix(from, target, t);
}

} // namespace kerf
