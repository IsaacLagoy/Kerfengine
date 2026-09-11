#version 330 core

in vec2 vUV;

out vec4 FragColor;

uniform sampler2D uAlbedo;
uniform vec4 uColor;

void main() {
    vec4 texel = texture(uAlbedo, vUV) * vec4(vUV.x, vUV.y, 0.0, 1.0);
    if (texel.a < 0.001) discard;
    FragColor = texel;
}
