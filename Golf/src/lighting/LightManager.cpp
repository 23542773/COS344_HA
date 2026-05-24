#include "LightManager.h"
#include "../shader.hpp"

LightManager::LightManager() 
    : isDay(true), timeOfDay(0.5f), dayNightFactor(0.0f), manualOverride(false) {  // ADD manualOverride initialization
    
    dayAmbient = glm::vec3(0.3f, 0.3f, 0.35f);
    nightAmbient = glm::vec3(0.05f, 0.05f, 0.1f);
    daySunDiffuse = glm::vec3(1.0f, 0.95f, 0.9f);
    nightSunDiffuse = glm::vec3(0.3f, 0.3f, 0.6f);
    
    currentSunAmbient = dayAmbient;
    currentSunDiffuse = daySunDiffuse;
    
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

void LightManager::setDayNightFactor(float factor) {
    // If manually overridden, don't auto-update
    if (manualOverride) return;
    
    dayNightFactor = glm::clamp(factor, 0.0f, 1.0f);
    updateSmoothLighting();
}

void LightManager::updateSmoothLighting() {
    // Interpolate sun lighting between day and night values
    currentSunAmbient = glm::mix(dayAmbient, nightAmbient, dayNightFactor);
    currentSunDiffuse = glm::mix(daySunDiffuse, nightSunDiffuse, dayNightFactor);
    
    sun.ambient = currentSunAmbient;
    sun.diffuse = currentSunDiffuse;
    
    // Fade point lights in/out smoothly based on night factor
    float pointLightIntensity = dayNightFactor;  // 0 at day, 1 at night
    
    for (auto& light : pointLights) {
        if (pointLightIntensity > 0.05f) {
            light.enabled = true;
            float intensityScale = glm::clamp(pointLightIntensity * 1.5f, 0.0f, 1.0f);
            light.diffuse = glm::mix(glm::vec3(0.0f), glm::vec3(0.9f, 0.8f, 0.7f), intensityScale);
        } else {
            light.enabled = false;
        }
    }
    
    // Update isDay flag based on factor (for skybox switching)
    bool newIsDay = (dayNightFactor < 0.5f);
    if (newIsDay != isDay) {
        isDay = newIsDay;
    }
}

void LightManager::setManualOverride(bool enabled) {
    manualOverride = enabled;
    if (!manualOverride) {
        updateSmoothLighting();
    }
}

void LightManager::resumeAutoCycle() {
    manualOverride = false;
    std::cout << "Auto cycle resumed" << std::endl;
}

void LightManager::setNightMode() {
    manualOverride = true;
    isDay = false;
    dayNightFactor = 1.0f;
    updateSmoothLighting();
    std::cout << "MANUAL OVERRIDE: Night mode set (auto cycle paused)" << std::endl;
}

void LightManager::setDayMode() {
    manualOverride = true;
    isDay = true;
    dayNightFactor = 0.0f;
    updateSmoothLighting();
    std::cout << "MANUAL OVERRIDE: Day mode set (auto cycle paused)" << std::endl;
}

void LightManager::updateDayNightCycle(float deltaTime, float speed) {
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

void LightManager::printLightState() const {
    std::cout << "\n=== LIGHTING STATE ===" << std::endl;
    std::cout << "Day/Night Factor: " << dayNightFactor << " (0=day, 1=night)" << std::endl;
    std::cout << "Time of Day: " << (isDay ? "DAY" : "NIGHT") << std::endl;
    std::cout << "Manual Override: " << (manualOverride ? "ON" : "OFF") << std::endl;
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