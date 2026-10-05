#include <kerf/engine/render/pipeline/TargetSize.h>


TargetSize::TargetSize(float scaleIn)
    : mode(Mode::Scale)
    , scale(scaleIn)
{
}

TargetSize::TargetSize(glm::ivec2 pixels)
    : mode(Mode::Pixels)
    , width(pixels.x)
    , height(pixels.y)
{
}

TargetSize TargetSize::fromScale(float scale)
{
    return TargetSize(scale);
}

TargetSize TargetSize::fromPixels(int width, int height)
{
    TargetSize size;
    size.mode = Mode::Pixels;
    size.width = width;
    size.height = height;
    return size;
}