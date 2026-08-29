#version 330 core
layout(location = 0) in vec3 aPos; // obj mesh has xyz but we only use xy
layout(location = 2) in vec2 aUV;  // obj server puts UVs at location 2 (location 1 is normal)

uniform mat3 uModel;
uniform mat4 uViewProjection;
uniform float uLayer;

out vec2 vUV;

void main() {
    // compute clip-space position
    vec3 world = uModel * vec3(aPos.xy, 1.0);
    vec4 clip = uViewProjection * vec4(world.xy, 0.0, 1.0);

    // use sigmoid to clamp layering to NDC manually
    float sigma = 1.0 / (1.0 + exp(-uLayer));
    float ndcZ = 1.0 - 2.0 * sigma;
    clip.z = ndcZ * clip.w;

    gl_Position = clip;
    vUV = aUV;
}
