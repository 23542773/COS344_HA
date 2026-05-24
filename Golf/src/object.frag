#version 330 core
out vec4 FragColor;

in vec2 TexCoords;
in vec3 Normal;
in vec3 FragPos;

uniform sampler2D texture_diffuse1;
uniform int useTexture;
uniform vec3 objectColor;

// Light structures
struct DirLight {
    vec3 direction;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    int enabled;
};

struct PointLight {
    vec3 position;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float constant;
    float linear;
    float quadratic;
    int enabled;
};

struct Spotlight {
    vec3 position;
    vec3 direction;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float cutOff;
    float outerCutOff;
    float constant;
    float linear;
    float quadratic;
    int enabled;
};

uniform DirLight dirLight;
uniform int numPointLights;
uniform PointLight pointLights[10];
uniform Spotlight spotlight;
uniform vec3 viewPos;

vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir) {
    if (light.enabled == 0) return vec3(0.0);
    
    vec3 lightDir = normalize(-light.direction);
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), 32.0);
    
    return (light.ambient + light.diffuse * diff + light.specular * spec);
}

vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir) {
    if (light.enabled == 0) return vec3(0.0);
    
    vec3 lightDir = normalize(light.position - fragPos);
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), 32.0);
    
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));
    
    return (light.ambient + light.diffuse * diff + light.specular * spec) * attenuation;
}

vec3 CalcSpotlight(Spotlight light, vec3 normal, vec3 fragPos, vec3 viewDir) {
    if (light.enabled == 0) return vec3(0.0);
    
    vec3 lightDir = normalize(light.position - fragPos);
    float theta = dot(lightDir, normalize(-light.direction));
    float epsilon = light.cutOff - light.outerCutOff;
    float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);
    
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), 32.0);
    
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));
    
    return (light.ambient + light.diffuse * diff + light.specular * spec) * attenuation * intensity;
}

void main() {
    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);
    
    vec3 lighting = vec3(0.0);
    
    lighting += CalcDirLight(dirLight, norm, viewDir);
    
    for (int i = 0; i < numPointLights; i++) {
        lighting += CalcPointLight(pointLights[i], norm, FragPos, viewDir);
    }
    
    lighting += CalcSpotlight(spotlight, norm, FragPos, viewDir);
    
    // Get the base color (either from texture or vertex color)
    vec3 baseColor;
    if (useTexture == 1) {
        baseColor = texture(texture_diffuse1, TexCoords).rgb;
    } else {
        baseColor = objectColor;
    }
    
    // Apply lighting to base color
    vec3 finalColor = baseColor * lighting;
    
    FragColor = vec4(finalColor, 1.0);
}