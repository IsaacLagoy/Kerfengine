#pragma once


namespace kerf {

enum class Format {
    RGBA8,
    RGB16F,
    Depth24
};

enum class Filter {
    Linear,
    Nearest
};

enum class AttachmentKind {
    Color,
    Normal,
    Depth
};

} // namespace kerf
