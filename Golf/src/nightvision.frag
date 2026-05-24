#version 330 core

in vec2 uv;
out vec4 FragColor;

uniform sampler2D screenTexture;
uniform bool nightMode;
uniform bool grayscale;

void main() {
    vec3 color = texture(screenTexture, uv).rgb;

    if (nightMode) {
        color *= 2.5;
        float green = dot(color, vec3(0.2, 0.7, 0.2));
        color = mix(color, vec3(0.1, 1.0, 0.2) * green, 0.7);
    }
    if (grayscale) {
        float gray = dot(color, vec3(0.299, 0.587, 0.114));
        color = vec3(gray);
    }

    FragColor = vec4(color, 1.0);
}
