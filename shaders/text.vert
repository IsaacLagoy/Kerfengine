#version 330 core

// Glyph quad in layout units (atlas pixels * displayScale).
layout(location = 0) in vec2 aPos;
layout(location = 1) in vec2 aUV;
layout(location = 2) in vec4 aColor;

// Same transform contract as default2d: world from Node, camera from Scene.
uniform mat3 uModel;
uniform mat4 uViewProjection;
uniform float uLayer;

out vec2 vUV;
out vec4 vColor;

void main() {
    vec3 world = uModel * vec3(aPos, 1.0);
    vec4 clip = uViewProjection * vec4(world.xy, 0.0, 1.0);

    // Manual NDC z from layer so text sorts with sprites (no depth pre-pass).
    float sigma = 1.0 / (1.0 + exp(-uLayer));
    float ndcZ = 1.0 - 2.0 * sigma;
    clip.z = ndcZ * clip.w;

    gl_Position = clip;
    vUV = aUV;
    vColor = aColor;
}
