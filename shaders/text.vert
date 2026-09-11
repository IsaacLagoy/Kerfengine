#version 330 core

// Glyph quad in layout units (atlas pixels * displayScale).
layout(location = 0) in vec2 aPos;
layout(location = 1) in vec2 aUV;
layout(location = 2) in vec4 aColor;

// Same transform contract as default2d: world from Node, camera from Scene.
uniform mat4 uModel;
uniform mat4 uViewProjection;
uniform float uLayer;

out vec2 vUV;
out vec4 vColor;

void main() {
    vec4 world = uModel * vec4(aPos, 0.0, 1.0);
    vec4 clip = uViewProjection * world;

    // tiny clip-space bias for layering
    clip.z += -uLayer * 1e-4 * clip.w;

    gl_Position = clip;
    vUV = aUV;
    vColor = aColor;
}
