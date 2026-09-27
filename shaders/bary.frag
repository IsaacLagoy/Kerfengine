#version 330 core

noperspective in vec3 vBary;

layout(location = 0) out vec4 oColor;
layout(location = 1) out vec4 oNormal;

void main() {
    float edge = min(min(vBary.x, vBary.y), vBary.z);
    float wire = 1.0 - smoothstep(0.0, fwidth(edge) * 5.0, edge);

    oColor = vec4(vBary * wire, 1.0);
    oNormal = vec4(0.0, 0.0, 1.0, 1.0);
}
