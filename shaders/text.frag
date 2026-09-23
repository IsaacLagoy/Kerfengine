#version 330 core

in vec2 vUV;
in vec4 vColor;

layout(location = 0) out vec4 oColor;
layout(location = 1) out vec4 oNormal;

uniform sampler2D uAtlas;

void main() {
    float alpha = texture(uAtlas, vUV).r;
    vec4 texel = vec4(vColor.rgb, vColor.a * alpha);
    if (texel.a < 0.001) discard;
    oColor = texel;
    oNormal = vec4(0.0, 0.0, 1.0, 1.0);
}
