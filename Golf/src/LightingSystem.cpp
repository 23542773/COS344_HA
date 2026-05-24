#include "LightingSystem.h"
#include <iostream>
#include <cmath>

LightingSystem::LightingSystem()
    : timeOfDay(0.5f), cycleSpeed(90.0f),  // 90 seconds for a full day/night cycle
      nightFactor(0.0f), spotlightEnabled(false), nightVisionEnabled(false) {
}

void LightingSystem::setupLights(GLuint shaderProgram) {
    // Directional Light (Sun) - initial values
    dirLightDirLoc = glGetUniformLocation(shaderProgram, "dirLight.direction");
    dirLightAmbientLoc = glGetUniformLocation(shaderProgram, "dirLight.ambient");
    dirLightDiffuseLoc = glGetUniformLocation(shaderProgram, "dirLight.diffuse");
    dirLightSpecularLoc = glGetUniformLocation(shaderProgram, "dirLight.specular");
    
    glUniform3f(dirLightDirLoc, -0.5f, -1.0f, -0.3f);
    glUniform3f(dirLightAmbientLoc, 0.3f, 0.3f, 0.35f);
    glUniform3f(dirLightDiffuseLoc, 1.0f, 0.95f, 0.9f);
    glUniform3f(dirLightSpecularLoc, 1.0f, 1.0f, 1.0f);
    
    // Point Lights (4 corners)
    glm::vec3 pointLightPositions[] = {
        glm::vec3(-40.0f, 5.0f, -30.0f),
        glm::vec3(40.0f, 5.0f, -30.0f),
        glm::vec3(40.0f, 5.0f, 30.0f),
        glm::vec3(-40.0f, 5.0f, 30.0f)
    };
    
    for (int i = 0; i < 4; i++) {
        std::string base = "pointLights[" + std::to_string(i) + "].";
        glUniform3f(glGetUniformLocation(shaderProgram, (base + "position").c_str()), 
                    pointLightPositions[i].x, pointLightPositions[i].y, pointLightPositions[i].z);
        glUniform3f(glGetUniformLocation(shaderProgram, (base + "ambient").c_str()), 0.1f, 0.1f, 0.1f);
        glUniform3f(glGetUniformLocation(shaderProgram, (base + "diffuse").c_str()), 1.0f, 0.9f, 0.7f);
        glUniform3f(glGetUniformLocation(shaderProgram, (base + "specular").c_str()), 0.5f, 0.5f, 0.5f);
        glUniform1f(glGetUniformLocation(shaderProgram, (base + "constant").c_str()), 1.0f);
        glUniform1f(glGetUniformLocation(shaderProgram, (base + "linear").c_str()), 0.09f);
        glUniform1f(glGetUniformLocation(shaderProgram, (base + "quadratic").c_str()), 0.032f);
        
        pointLightDiffuseLocs[i] = glGetUniformLocation(shaderProgram, (base + "diffuse").c_str());
    }
    
    // Spotlight (Drone)
    spotLightPosLoc = glGetUniformLocation(shaderProgram, "spotLight.position");
    spotLightDirLoc = glGetUniformLocation(shaderProgram, "spotLight.direction");
    spotLightDiffuseLoc = glGetUniformLocation(shaderProgram, "spotLight.diffuse");
    spotLightSpecularLoc = glGetUniformLocation(shaderProgram, "spotLight.specular");
    
    glUniform3f(glGetUniformLocation(shaderProgram, "spotLight.ambient"), 0.05f, 0.05f, 0.05f);
    glUniform3f(spotLightDiffuseLoc, 1.0f, 1.0f, 1.0f);
    glUniform3f(spotLightSpecularLoc, 1.0f, 1.0f, 1.0f);
    glUniform1f(glGetUniformLocation(shaderProgram, "spotLight.constant"), 1.0f);
    glUniform1f(glGetUniformLocation(shaderProgram, "spotLight.linear"), 0.09f);
    glUniform1f(glGetUniformLocation(shaderProgram, "spotLight.quadratic"), 0.032f);
    glUniform1f(glGetUniformLocation(shaderProgram, "spotLight.cutOff"), cos(glm::radians(25.0f)));
    glUniform1f(glGetUniformLocation(shaderProgram, "spotLight.outerCutOff"), cos(glm::radians(35.0f)));
    
    // Material properties
    glUniform1f(glGetUniformLocation(shaderProgram, "shininess"), 32.0f);
    
    // Uniform locations for per-frame updates
    viewPosLoc = glGetUniformLocation(shaderProgram, "viewPos");
    nightFactorLoc = glGetUniformLocation(shaderProgram, "nightFactor");
    nightVisionLoc = glGetUniformLocation(shaderProgram, "nightVision");
}

void LightingSystem::updateSunLight(GLuint shaderProgram, float deltaTime) {
    // Day/Night cycle progression - use deltaTime for smooth timing
    timeOfDay += deltaTime / cycleSpeed;
    if (timeOfDay > 1.0f) timeOfDay -= 1.0f;
    
    // Calculate night factor (0 = day, 1 = night)
    nightFactor = sin(timeOfDay * 3.14159f);
    nightFactor = std::max(0.0f, std::min(1.0f, nightFactor * 1.5f - 0.5f));
    
    // Sun colour based on night factor
    glm::vec3 sunColor;
    float sunIntensity;
    
    if (nightFactor < 0.5f) {
        // DAYTIME
        float t = nightFactor * 2.0f;
        sunColor = glm::mix(glm::vec3(1.0f, 0.95f, 0.85f), glm::vec3(1.0f, 0.98f, 0.92f), t);
        sunIntensity = 1.2f;
    } else {
        // NIGHTTIME
        float t = (nightFactor - 0.5f) * 2.0f;
        sunColor = glm::mix(glm::vec3(1.0f, 0.5f, 0.2f), glm::vec3(0.3f, 0.3f, 0.6f), t);
        sunIntensity = 0.15f;
    }
    
    // Sun direction based on time of day
    float sunAngle = (timeOfDay - 0.25f) * 2.0f * 3.14159f;
    glm::vec3 sunDir = glm::normalize(glm::vec3(cos(sunAngle), sin(sunAngle), 0.3f));
    
    glUniform3f(dirLightDirLoc, -sunDir.x, -sunDir.y, -sunDir.z);
    glUniform3f(dirLightDiffuseLoc, sunColor.x * sunIntensity, sunColor.y * sunIntensity, sunColor.z * sunIntensity);
    glUniform3f(dirLightAmbientLoc, 0.3f * sunIntensity, 0.3f * sunIntensity, 0.35f * sunIntensity);
}

void LightingSystem::updatePointLights(GLuint shaderProgram) {
    // Point lights: OFF during day, ON during night
    float pointIntensity = nightFactor > 0.5f ? 0.8f : 0.05f;
    
    for (int i = 0; i < 4; i++) {
        glUniform3f(pointLightDiffuseLocs[i], 
                    1.0f * pointIntensity, 0.9f * pointIntensity, 0.7f * pointIntensity);
    }
}

void LightingSystem::updateSpotlight(GLuint shaderProgram, const glm::vec3& cameraPos, 
                                      const glm::vec3& cameraFront) {
    float spotIntensity = spotlightEnabled ? 1.0f : 0.0f;
    
    glUniform3f(spotLightPosLoc, cameraPos.x, cameraPos.y, cameraPos.z);
    glUniform3f(spotLightDirLoc, cameraFront.x, cameraFront.y, cameraFront.z);
    glUniform3f(spotLightDiffuseLoc, spotIntensity, spotIntensity, spotIntensity);
    glUniform3f(spotLightSpecularLoc, spotIntensity, spotIntensity, spotIntensity);
}

void LightingSystem::updateLights(GLuint shaderProgram, const glm::vec3& cameraPos,
                                   const glm::vec3& cameraFront, float deltaTime,
                                   bool spotlightOn, bool nightVisionOn) {
    // Update internal state from external toggles
    spotlightEnabled = spotlightOn;
    nightVisionEnabled = nightVisionOn;
    
    // Update all light components
    updateSunLight(shaderProgram, deltaTime);  // Pass deltaTime here
    updatePointLights(shaderProgram);
    updateSpotlight(shaderProgram, cameraPos, cameraFront);
    
    // Bind view position, night factor, and night vision
    glUniform3f(viewPosLoc, cameraPos.x, cameraPos.y, cameraPos.z);
    glUniform1f(nightFactorLoc, nightFactor);
    glUniform1i(nightVisionLoc, nightVisionEnabled ? 1 : 0);
}