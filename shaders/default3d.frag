#version 330 core

in vec2 vUV;
in vec3 vNormal;

layout(location = 0) out vec4 oColor;
layout(location = 1) out vec4 oNormal;

uniform sampler2D uAlbedo;
uniform vec4 uColor;

void main() {
    vec4 albedo = texture(uAlbedo, vUV);
    vec3 rgb = albedo.rgb * uColor.rgb;
    float alpha = albedo.a * uColor.a;
    if (alpha < 0.001) discard;

    vec3 n = normalize(vNormal);
    // vec3 lightDir = normalize(vec3(0.4, 0.8, 0.6));
    // float lighting = 0.35 + 0.65 * max(dot(n, lightDir), 0.0);

    oColor = vec4(rgb * alpha, alpha);
    oNormal = vec4(n, 1.0);
}
