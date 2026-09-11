#version 330 core
layout(location = 0) in vec3 aPos; // obj mesh has xyz but we only use xy
layout(location = 2) in vec2 aUV;  // obj server puts UVs at location 2 (location 1 is normal)

uniform mat4 uModel;
uniform mat4 uViewProjection;
uniform float uLayer;

out vec2 vUV;

void main() {
    // compute clip-space position
    vec4 world = uModel * vec4(aPos.xy, 0.0, 1.0);
    vec4 clip = uViewProjection * world;

    // tiny clip-space bias for layering
    clip.z += -uLayer * 1e-4 * clip.w;

    gl_Position = clip;
    vUV = aUV;
}
