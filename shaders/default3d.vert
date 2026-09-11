#version 330 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aUV;

uniform mat4 uModel;
uniform mat4 uViewProjection;

out vec2 vUV;
out vec3 vNormal;

void main() {
    vec4 world = uModel * vec4(aPos, 1.0);
    gl_Position = uViewProjection * world;
    vUV = aUV;
    vNormal = mat3(uModel) * aNormal;
}
