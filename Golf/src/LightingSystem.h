#ifndef LIGHTING_SYSTEM_H
#define LIGHTING_SYSTEM_H

#include <GL/glew.h>
#include <glm/glm.hpp>

class LightingSystem {
public:
    LightingSystem();
    
    void setupLights(GLuint shaderProgram);
    void updateLights(GLuint shaderProgram, const glm::vec3& cameraPos, 
                      const glm::vec3& cameraFront, float deltaTime, 
                      bool spotlightOn, bool nightVisionOn);
    
    // Getters for UI
    float getNightFactor() const { return nightFactor; }
    float getTimeOfDay() const { return timeOfDay; }
    bool isNightTime() const { return nightFactor > 0.5f; }
    
    // Toggle functions
    void toggleSpotlight() { spotlightEnabled = !spotlightEnabled; }
    bool isSpotlightOn() const { return spotlightEnabled; }
    
    void toggleNightVision() { nightVisionEnabled = !nightVisionEnabled; }
    bool isNightVisionOn() const { return nightVisionEnabled; }
    
    void resetTimeOfDay() { timeOfDay = 0.5f; }
    
private:
    float timeOfDay;
    float cycleSpeed;
    float nightFactor;
    bool spotlightEnabled;
    bool nightVisionEnabled;
    
    // Light uniform locations
    GLuint dirLightDirLoc;
    GLuint dirLightAmbientLoc;
    GLuint dirLightDiffuseLoc;
    GLuint dirLightSpecularLoc;
    
    GLuint spotLightPosLoc;
    GLuint spotLightDirLoc;
    GLuint spotLightDiffuseLoc;
    GLuint spotLightSpecularLoc;
    
    GLuint pointLightDiffuseLocs[4];
    
    GLuint viewPosLoc;
    GLuint nightFactorLoc;
    GLuint nightVisionLoc;
    
    void updateSunLight(GLuint shaderProgram, float deltaTime);  // Added deltaTime parameter
    void updatePointLights(GLuint shaderProgram);
    void updateSpotlight(GLuint shaderProgram, const glm::vec3& cameraPos, 
                         const glm::vec3& cameraFront);
};

#endif