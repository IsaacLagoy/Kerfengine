#version 330 core
layout(location = 0) in vec3 aPos;
layout(location = 2) in vec2 aUV;

out vec2 vUV;

void main() {
    // unit.obj is -0.5..0.5; map to NDC
    gl_Position = vec4(aPos.xy * 2.0, 0.0, 1.0);
    vUV = aUV;
}
