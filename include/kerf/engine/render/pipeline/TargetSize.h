#pragma once

#include <glm/glm.hpp>


struct TargetSize {
    enum class Mode {
        Scale,
        Pixels
    };

    Mode mode = Mode::Scale;
    float scale = 1.0f;
    int width = 0;
    int height = 0;

    TargetSize() = default;
    explicit TargetSize(float scaleIn);
    TargetSize(glm::ivec2 pixels);

    // factories
    static TargetSize fromScale(float scale);
    static TargetSize fromPixels(int width, int height);
    static TargetSize fromWidth(int width);
};