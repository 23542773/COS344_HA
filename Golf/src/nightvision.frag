#version 330 core

in vec2 uv;
out vec4 FragColor;

uniform sampler2D screenTexture;
uniform bool nightMode;

void main() {
    vec3 color = texture(screenTexture, uv).rgb;

    if (nightMode) {
        color *= 1.8;

        float green = dot(color, vec3(0.2, 0.7, 0.2));
        color = mix(color, vec3(0.1, 1.0, 0.2) * green, 0.7);
    }

    FragColor = vec4(color, 1.0);
}
