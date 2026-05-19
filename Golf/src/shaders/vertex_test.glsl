#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

out vec3 FragNormal;
out vec2 TexCoords;

uniform mat4 uModel;

void main()
{
    // Output the local coordinate transformations directly
    gl_Position = uModel * vec4(aPos, 1.0);
    
    // Pass the vertex normals and UVs down the pipeline
    FragNormal = aNormal;
    TexCoords = aTexCoords;
}