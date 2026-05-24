#include "LightManager.h"
#include "../shader.hpp"  // Add this include

LightManager::LightManager() 
    : isDay(true), timeOfDay(0.5f) {
    
    dayAmbient = glm::vec3(0.3f, 0.3f, 0.35f);
    nightAmbient = glm::vec3(0.05f, 0.05f, 0.1f);
    daySunDiffuse = glm::vec3(1.0f, 0.95f, 0.9f);
    nightSunDiffuse = glm::vec3(0.3f, 0.3f, 0.6f);
    
    initCourseLights();
    updateLightColours();
}

void LightManager::initCourseLights() {
    pointLights.clear();
    
    // Corner 1: Top-left
    pointLights.emplace_back(glm::vec3(-35.0f, 2.0f, -20.0f), 
                             glm::vec3(0.1f), glm::vec3(1.0f, 0.9f, 0.7f), glm::vec3(0.5f));
    
    // Corner 2: Top-right
    pointLights.emplace_back(glm::vec3(35.0f, 2.0f, -20.0f),
                             glm::vec3(0.1f), glm::vec3(1.0f, 0.9f, 0.7f), glm::vec3(0.5f));
    
    // Corner 3: Bottom-left
    pointLights.emplace_back(glm::vec3(-35.0f, 2.0f, 20.0f),
                             glm::vec3(0.1f), glm::vec3(1.0f, 0.9f, 0.7f), glm::vec3(0.5f));
    
    // Corner 4: Bottom-right
    pointLights.emplace_back(glm::vec3(35.0f, 2.0f, 20.0f),
                             glm::vec3(0.1f), glm::vec3(1.0f, 0.9f, 0.7f), glm::vec3(0.5f));
    
    // Additional light near water feature
    pointLights.emplace_back(glm::vec3(0.0f, 1.5f, 0.0f),
                             glm::vec3(0.1f), glm::vec3(0.8f, 0.9f, 1.0f), glm::vec3(0.5f));
}

void LightManager::updateLightColours() {
    if (isDay) {
        sun.ambient = dayAmbient;
        sun.diffuse = daySunDiffuse;
        sun.direction = glm::normalize(glm::vec3(0.5f, -1.0f, 0.3f));
        
        for (auto& light : pointLights) {
            light.enabled = false;
        }
    } else {
        sun.ambient = nightAmbient;
        sun.diffuse = nightSunDiffuse;
        sun.direction = glm::normalize(glm::vec3(0.1f, -0.3f, 0.2f));
        
        for (auto& light : pointLights) {
            light.enabled = true;
        }
    }
}

void LightManager::updateDayNightCycle(float deltaTime, float speed) {
    // Optional smooth transition
    (void)deltaTime;
    (void)speed;
}

void LightManager::bindAllLights(Shader& shader) const {
    shader.use();
    
    sun.bindToShader(shader);
    
    int numLights = 0;
    for (const auto& light : pointLights) {
        if (light.enabled && numLights < 10) {
            std::string prefix = "pointLights[" + std::to_string(numLights) + "].";
            light.bindToShader(shader, prefix);
            numLights++;
        }
    }
    shader.setInt("numPointLights", numLights);
    
    droneLight.bindToShader(shader);
}

void LightManager::setSunDirection(const glm::vec3& dir) {
    sun.direction = glm::normalize(dir);
}

void LightManager::toggleSun() {
    sun.enabled = !sun.enabled;
}

void LightManager::togglePointLight(int index) {
    if (index < static_cast<int>(pointLights.size())) {
        pointLights[index].enabled = !pointLights[index].enabled;
    }
}

void LightManager::toggleDroneLight() {
    droneLight.enabled = !droneLight.enabled;
}

void LightManager::setNightMode() {
    isDay = false;
    updateLightColours();
}

void LightManager::setDayMode() {
    isDay = true;
    updateLightColours();
}

void LightManager::printLightState() const {
    std::cout << "\n=== LIGHTING STATE ===" << std::endl;
    std::cout << "Time of Day: " << (isDay ? "DAY" : "NIGHT") << std::endl;
    std::cout << "Sun: " << (sun.enabled ? "ON" : "OFF");
    std::cout << " Direction: (" << sun.direction.x << ", " << sun.direction.y << ", " << sun.direction.z << ")" << std::endl;
    std::cout << "Sun Ambient: (" << sun.ambient.x << ", " << sun.ambient.y << ", " << sun.ambient.z << ")" << std::endl;
    std::cout << "Sun Diffuse: (" << sun.diffuse.x << ", " << sun.diffuse.y << ", " << sun.diffuse.z << ")" << std::endl;
    
    int activePointLights = 0;
    for (const auto& light : pointLights) {
        if (light.enabled) activePointLights++;
    }
    std::cout << "Active Point Lights: " << activePointLights << "/" << pointLights.size() << std::endl;
    std::cout << "Drone Light: " << (droneLight.enabled ? "ON" : "OFF") << std::endl;
    std::cout << "========================\n" << std::endl;
}

// In LightManager.cpp, add this function:
void LightManager::setDroneLightIntensity(float intensity) {
    droneLight.diffuse = glm::vec3(intensity, intensity, intensity);
    droneLight.ambient = glm::vec3(intensity * 0.15f, intensity * 0.15f, intensity * 0.15f);
}