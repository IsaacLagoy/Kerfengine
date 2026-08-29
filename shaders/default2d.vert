#version 330 core
layout(location = 0) in vec3 aPos; // Mesh stores xyz; 2D uses xy
layout(location = 2) in vec2 aUV;  // ObjServer puts UVs at location 2

uniform mat3 uModel;
uniform mat4 uViewProjection;
uniform float uLayer;

out vec2 vUV;

void main() {
    vec3 world = uModel * vec3(aPos.xy, 1.0);
    vec4 clip = uViewProjection * vec4(world.xy, 0.0, 1.0);

    // Higher layer → closer. σ:(−∞,∞)→(0,1), then to NDC z (−1,1).
    // Useful precision is roughly layer ∈ [−6, 6]; beyond that the curve flattens.
    float sigma = 1.0 / (1.0 + exp(-uLayer));
    float ndcZ = 1.0 - 2.0 * sigma;
    clip.z = ndcZ * clip.w;

    gl_Position = clip;
    vUV = aUV;
}
