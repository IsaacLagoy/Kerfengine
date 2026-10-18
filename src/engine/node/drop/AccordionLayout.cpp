#include <kerf/engine/node/drop/AccordionLayout.h>

#include <algorithm>
#include <cmath>


namespace kerf::accordion {

float slotOffset(int index, int count, float length)
{
    if (count <= 0)
    {
        return 0.0f;
    }
    const float share = length / static_cast<float>(count);
    return -0.5f * length + (static_cast<float>(index) + 0.5f) * share;
}

int slotAt(float cursor, int count, float length)
{
    if (count <= 1 || length <= 0.0f)
    {
        return 0;
    }
    const float share = length / static_cast<float>(count);
    const int slot = static_cast<int>(std::floor((cursor + 0.5f * length) / share));
    return std::clamp(slot, 0, count - 1);
}

} // namespace kerf::accordion
