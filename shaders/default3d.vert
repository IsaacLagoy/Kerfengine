#version 330 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aUV;
layout(location = 3) in vec3 aBary;

uniform mat4 uModel;
uniform mat4 uViewProjection;

out vec2 vUV;
out vec3 vNormal;
noperspective out vec3 vBary;

void main() {
    vec4 world = uModel * vec4(aPos, 1.0);
    gl_Position = uViewProjection * world;
    vUV = aUV;
    vNormal = mat3(transpose(inverse(uModel))) * aNormal;
    vBary = aBary;
}
