#version 330 core

in vec2 vUV;
in vec4 vColor;

out vec4 FragColor;

// Font atlas: GL_R8 coverage. Interior of a glyph is ~1, empty is 0.
uniform sampler2D uAtlas;

void main() {
    float alpha = texture(uAtlas, vUV).r;
    vec4 texel = vec4(vColor.rgb, vColor.a * alpha);
    if (texel.a < 0.001) discard;
    FragColor = texel;
}
