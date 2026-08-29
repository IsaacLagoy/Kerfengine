#version 330 core
layout(location=0) in vec2 aPos;
layout(location=1) in vec2 aUV;
uniform mat4 uProjection;
uniform vec2 uOrigin; // top-left of the text box, in screen pixels
out vec2 vUV;
void main() {
    vec2 world = aPos + uOrigin;
    gl_Position = uProjection * vec4(world, 0.0, 1.0);
    vUV = aUV;
}
