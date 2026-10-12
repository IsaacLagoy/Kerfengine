#version 330 core

in vec2 vUV;

layout(location = 0) out vec4 oColor;
layout(location = 1) out vec4 oNormal;

uniform sampler2D uAlbedo;
uniform vec4 uColor;

void main() {
    vec4 albedo = texture(uAlbedo, vUV);
    vec3 rgb = albedo.rgb * uColor.rgb;
    float alpha = albedo.a * uColor.a;
    if (alpha < 0.001) discard;
    oColor = vec4(rgb * alpha, alpha);
    oNormal = vec4(0.0, 0.0, 1.0, 1.0);
}
