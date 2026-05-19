#version 330 core

in vec3 FragNormal;
in vec2 TexCoords;

out vec4 FragColor;

void main()
{
    // Normalize vectors to account for interpolation shifts
    vec3 norm = normalize(FragNormal);
    
    // Transform normal data from [-1, 1] space into visible [0, 1] color components
    vec3 diagnosticColor = norm * 0.5 + 0.5;
    
    // Output the visual depth coloring map
    FragColor = vec4(diagnosticColor, 1.0);
}