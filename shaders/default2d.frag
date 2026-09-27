#version 330 core

in vec2 vUV;

layout(location = 0) out vec4 oColor;
layout(location = 1) out vec4 oNormal;

uniform sampler2D uAlbedo;
uniform vec4 uColor;

void main() {
    vec4 texel = texture(uAlbedo, vUV) * uColor;
    if (texel.a < 0.001) discard;
    oColor = texel;
    oNormal = vec4(0.0, 0.0, 1.0, 1.0);
}
