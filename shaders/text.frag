#version 330 core
in vec2 vUV;
out vec4 FragColor;
uniform sampler2D uAtlas;
uniform vec3 uColor;
void main() {
    float alpha = texture(uAtlas, vUV).r; // atlas is single-channel coverage
    FragColor = vec4(uColor, alpha);
}
