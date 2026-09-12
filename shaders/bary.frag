#version 330 core

noperspective in vec3 vBary;

out vec4 FragColor;

void main() {
    float edge = min(min(vBary.x, vBary.y), vBary.z);
    float wire = 1.0 - smoothstep(0.0, fwidth(edge) * 5.0, edge);

    FragColor = vec4(vBary * wire, 1.0);
}
