#include "Light.h"

// Light base class implementation
Light::Light() : ambient(0.1f), diffuse(0.8f), specular(0.5f), enabled(true) {}

// DirectionalLight implementation
DirectionalLight::DirectionalLight() 
    : direction(glm::vec3(-0.5f, -1.0f, -0.3f)) {
    ambient = glm::vec3(0.3f, 0.3f, 0.35f);
    diffuse = glm::vec3(1.0f, 0.95f, 0.9f);
    specular = glm::vec3(1.0f, 1.0f, 1.0f);
}

void DirectionalLight::bindToShader(Shader& shader, const std::string& prefix) const {
    shader.setVec3(prefix + ".direction", direction);
    shader.setVec3(prefix + ".ambient", ambient);
    shader.setVec3(prefix + ".diffuse", diffuse);
    shader.setVec3(prefix + ".specular", specular);
    shader.setInt(prefix + ".enabled", enabled ? 1 : 0);
}

void DirectionalLight::updateLightSpaceMatrix(float left, float right, float bottom, float top,
                                                float nearPlane, float farPlane) {
    glm::mat4 lightProjection = glm::ortho(left, right, bottom, top, nearPlane, farPlane);
    glm::mat4 lightView = glm::lookAt(
        -direction * 50.0f,
        glm::vec3(0.0f),
        glm::vec3(0.0f, 1.0f, 0.0f)
    );
    lightSpaceMatrix = lightProjection * lightView;
}

// PointLight implementation
PointLight::PointLight() 
    : position(glm::vec3(0.0f)), constant(1.0f), linear(0.09f), quadratic(0.032f) {
    ambient = glm::vec3(0.1f);
    diffuse = glm::vec3(0.9f, 0.8f, 0.7f);
    specular = glm::vec3(0.5f);
}

PointLight::PointLight(glm::vec3 pos, glm::vec3 amb, glm::vec3 diff, glm::vec3 spec)
    : position(pos), constant(1.0f), linear(0.09f), quadratic(0.032f) {
    ambient = amb;
    diffuse = diff;
    specular = spec;
}

void PointLight::bindToShader(Shader& shader, const std::string& prefix) const {
    shader.setVec3(prefix + ".position", position);
    shader.setVec3(prefix + ".ambient", ambient);
    shader.setVec3(prefix + ".diffuse", diffuse);
    shader.setVec3(prefix + ".specular", specular);
    shader.setFloat(prefix + ".constant", constant);
    shader.setFloat(prefix + ".linear", linear);
    shader.setFloat(prefix + ".quadratic", quadratic);
    shader.setInt(prefix + ".enabled", enabled ? 1 : 0);
}

// Spotlight implementation
Spotlight::Spotlight()
    : position(glm::vec3(0.0f)), direction(glm::vec3(0.0f, -1.0f, 0.0f)),
      cutOff(glm::cos(glm::radians(45.0f))),       // 45 degree inner cone
      outerCutOff(glm::cos(glm::radians(65.0f))),  // 65 degree outer cone
      constant(1.0f), linear(0.09f), quadratic(0.032f) {
    ambient = glm::vec3(0.3f, 0.3f, 0.3f);    // Higher ambient
    diffuse = glm::vec3(2.0f, 2.0f, 2.0f);     // Brighter diffuse
    specular = glm::vec3(1.0f, 1.0f, 1.0f);
    enabled = true;
}

void Spotlight::bindToShader(Shader& shader, const std::string& prefix) const {
    shader.setVec3(prefix + ".position", position);
    shader.setVec3(prefix + ".direction", direction);
    shader.setVec3(prefix + ".ambient", ambient);
    shader.setVec3(prefix + ".diffuse", diffuse);
    shader.setVec3(prefix + ".specular", specular);
    shader.setFloat(prefix + ".cutOff", cutOff);
    shader.setFloat(prefix + ".outerCutOff", outerCutOff);
    shader.setFloat(prefix + ".constant", constant);
    shader.setFloat(prefix + ".linear", linear);
    shader.setFloat(prefix + ".quadratic", quadratic);
    shader.setInt(prefix + ".enabled", enabled ? 1 : 0);
}