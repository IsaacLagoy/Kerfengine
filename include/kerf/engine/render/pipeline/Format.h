#pragma once


namespace kerf {

enum class Format {
    RGBA8,
    RGB16F,
    Depth24,
    UnsignedInt8
};

enum class Filter {
    Linear,
    Nearest
};

} // namespace kerf
