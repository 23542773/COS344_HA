#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

void main() {
    // A flat, artificial turf green
    FragColor = vec4(0.20, 0.60, 0.25, 1.0); 
}