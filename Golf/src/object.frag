#version 330 core

in vec2 TexCoord;
in vec3 FragPos;
in vec3 Normal;

out vec4 FragColor;

// ========== LIGHT STRUCTS ==========

struct DirLight {
    vec3 direction;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct PointLight {
    vec3 position;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float constant;
    float linear;
    float quadratic;
};

struct SpotLight {
    vec3 position;
    vec3 direction;
    float cutOff;
    float outerCutOff;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float constant;
    float linear;
    float quadratic;
};

// ========== UNIFORMS ==========

uniform vec3 viewPos;
uniform sampler2D texture1;
uniform bool useTexture;
uniform vec3 objectColor;

// Lights
uniform DirLight dirLight;
uniform PointLight pointLights[4];
uniform SpotLight spotLight;

// Day/Night factor (0 = day, 1 = night)
uniform float nightFactor;

// Material properties
uniform float shininess;

// Night vision toggle
uniform bool nightVision;

// ========== FUNCTION PROTOTYPES ==========

vec3 calcDirLight(DirLight light, vec3 normal, vec3 viewDir, vec3 diffuseColor, vec3 specularColor);
vec3 calcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec3 diffuseColor, vec3 specularColor);
vec3 calcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec3 diffuseColor, vec3 specularColor);

void main()
{
    // Sample texture or use object color
    vec3 diffuseColor;
    vec3 specularColor = vec3(0.5, 0.5, 0.5);
    
    if (useTexture) {
        diffuseColor = vec3(texture(texture1, TexCoord));
    } else {
        diffuseColor = objectColor;
    }
    
    vec3 normal = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);
    
    // Calculate lighting
    vec3 result = calcDirLight(dirLight, normal, viewDir, diffuseColor, specularColor);
    
    for (int i = 0; i < 4; i++) {
        result += calcPointLight(pointLights[i], normal, FragPos, viewDir, diffuseColor, specularColor);
    }
    
    result += calcSpotLight(spotLight, normal, FragPos, viewDir, diffuseColor, specularColor);
    
    // Apply night factor (reduce brightness at night)
    result *= (1.0 - nightFactor * 0.6);
    
    // ========== NIGHT VISION EFFECT ==========
    if (nightVision) {
        // Amplify brightness for dark areas
        result *= 2.5;
        
        // Apply green phosphor tint (classic night vision look)
        result = vec3(result.g * 1.2, result.g, result.g * 0.8);
    }
    
    FragColor = vec4(result, 1.0);
}

// ========== LIGHT CALCULATIONS ==========

vec3 calcDirLight(DirLight light, vec3 normal, vec3 viewDir, vec3 diffuseColor, vec3 specularColor)
{
    vec3 lightDir = normalize(-light.direction);
    
    // Diffuse
    float diff = max(dot(normal, lightDir), 0.0);
    
    // Specular (Blinn-Phong)
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), shininess);
    
    vec3 ambient = light.ambient * diffuseColor;
    vec3 diffuse = light.diffuse * diff * diffuseColor;
    vec3 specular = light.specular * spec * specularColor;
    
    return ambient + diffuse + specular;
}

vec3 calcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec3 diffuseColor, vec3 specularColor)
{
    vec3 lightDir = normalize(light.position - fragPos);
    
    // Diffuse
    float diff = max(dot(normal, lightDir), 0.0);
    
    // Specular (Blinn-Phong)
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), shininess);
    
    // Attenuation
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));
    
    vec3 ambient = light.ambient * diffuseColor;
    vec3 diffuse = light.diffuse * diff * diffuseColor;
    vec3 specular = light.specular * spec * specularColor;
    
    return (ambient + diffuse + specular) * attenuation;
}

vec3 calcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec3 diffuseColor, vec3 specularColor)
{
    vec3 lightDir = normalize(light.position - fragPos);
    
    // Diffuse
    float diff = max(dot(normal, lightDir), 0.0);
    
    // Specular
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), shininess);
    
    // Attenuation
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));
    
    // Spotlight intensity (soft edge)
    float theta = dot(lightDir, normalize(-light.direction));
    float epsilon = light.cutOff - light.outerCutOff;
    float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);
    
    vec3 ambient = light.ambient * diffuseColor;
    vec3 diffuse = light.diffuse * diff * diffuseColor;
    vec3 specular = light.specular * spec * specularColor;
    
    return (ambient + diffuse + specular) * attenuation * intensity;
}