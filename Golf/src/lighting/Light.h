#ifndef LIGHT_H
#define LIGHT_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <string>
#include "../shader.hpp"

// Base light structure
struct Light {
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;
    bool enabled;
    
    Light();
};

// Directional light (sun)
struct DirectionalLight : public Light {
    glm::vec3 direction;
    glm::mat4 lightSpaceMatrix;
    
    DirectionalLight();
    void bindToShader(Shader& shader, const std::string& prefix = "dirLight") const;
    void updateLightSpaceMatrix(float left, float right, float bottom, float top, 
                                 float nearPlane, float farPlane);
};

// Point light (course lighting)
struct PointLight : public Light {
    glm::vec3 position;
    float constant;
    float linear;
    float quadratic;
    
    PointLight();
    PointLight(glm::vec3 pos, glm::vec3 amb, glm::vec3 diff, glm::vec3 spec);
    void bindToShader(Shader& shader, const std::string& prefix) const;
};

// Spotlight (drone headlight)
struct Spotlight : public Light {
    glm::vec3 position;
    glm::vec3 direction;
    float cutOff;
    float outerCutOff;
    float constant;
    float linear;
    float quadratic;
    
    Spotlight();
    void bindToShader(Shader& shader, const std::string& prefix = "spotlight") const;
};

#endif