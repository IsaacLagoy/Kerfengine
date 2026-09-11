#version 330 core

in vec2 vUV;
in vec3 vNormal;

out vec4 FragColor;

uniform sampler2D uAlbedo;
uniform vec4 uColor;

void main() {
    vec4 texel = texture(uAlbedo, vUV) * uColor;
    if (texel.a < 0.001) discard;

    vec3 n = normalize(vNormal);
    vec3 lightDir = normalize(vec3(0.4, 0.8, 0.6));
    float lighting = 0.35 + 0.65 * max(dot(n, lightDir), 0.0);

    FragColor = vec4(texel.rgb * lighting, texel.a);
}
