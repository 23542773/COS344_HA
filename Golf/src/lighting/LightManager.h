#ifndef LIGHT_MANAGER_H
#define LIGHT_MANAGER_H

#include "Light.h"
#include <vector>
#include <iostream>

// Forward declaration
class Shader;

class LightManager {
private:
    DirectionalLight sun;
    std::vector<PointLight> pointLights;
    Spotlight droneLight;
    
    bool isDay;
    float timeOfDay;
    
    glm::vec3 dayAmbient;
    glm::vec3 nightAmbient;
    glm::vec3 daySunDiffuse;
    glm::vec3 nightSunDiffuse;
    
public:
    LightManager();
    ~LightManager() = default;
    
    void initCourseLights();
    void updateDayNightCycle(float deltaTime, float speed = 0.1f);
    void bindAllLights(Shader& shader) const;
    
    // Getters
    DirectionalLight& getSun() { return sun; }
    const DirectionalLight& getSun() const { return sun; }
    std::vector<PointLight>& getPointLights() { return pointLights; }
    const std::vector<PointLight>& getPointLights() const { return pointLights; }
    Spotlight& getDroneLight() { return droneLight; }
    const Spotlight& getDroneLight() const { return droneLight; }
    bool getIsDay() const { return isDay; }
    
    // Setters for runtime adjustments
    void setSunDirection(const glm::vec3& dir);
    void toggleSun();
    void togglePointLight(int index);
    void toggleDroneLight();
    void setNightMode();
    void setDayMode();
    
    // Debug functions
    void printLightState() const;
    void setDroneLightIntensity(float intensity);
private:
    void updateLightColours();
};

#endif