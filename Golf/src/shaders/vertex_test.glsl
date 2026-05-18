#version 330 core

// Layout mapping locations matched to your ObjLoader.h specifications
layout (location = 0) in vec3 aPos;
layout (location = 2) in vec3 aNormal;

uniform mat4 uModel;

void main() {
    // Simple 0.1 scale division transformation to fit the unprojected test camera box
    gl_Position = uModel * vec4(aPos * 0.1, 1.0);
}