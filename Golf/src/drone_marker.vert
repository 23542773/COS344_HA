#version 330 core

layout(location = 0) in vec2 aPos;

uniform vec2 uOffset;   // screen position in pixels
uniform vec2 uScale;    // size in pixels
uniform vec2 uResolution;

void main()
{
    vec2 pos = aPos * uScale + uOffset;

    // convert from pixels → NDC
    vec2 ndc = (pos / uResolution) * 2.0 - 1.0;

    gl_Position = vec4(ndc, 0.0, 1.0);
}
