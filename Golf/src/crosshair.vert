#version 330 core

layout(location = 0) in vec2 aPos;

uniform vec2 uResolution;
uniform vec2 uCenter;

void main() {

    vec2 screenPos = aPos + uCenter;

    vec2 ndc = (screenPos / uResolution) * 2.0 - 1.0;

    gl_Position = vec4(ndc.x, ndc.y, 0.0, 1.0);
}
