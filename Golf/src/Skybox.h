#ifndef SKYBOX_H
#define SKYBOX_H

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <vector>
#include <string>

class Skybox {

    public:
        Skybox(std::vector<std::string> dayFaces, std::vector<std::string> nightFaces);
        ~Skybox();

        void draw(glm::mat4 view, glm::mat4 projection, bool isNight);
    
    private:
       unsigned int skyboxVAO, skyboxVBO;
       unsigned int dayCubemapTexture;
       unsigned int nightCubemapTexture; 

       unsigned int shaderProgram;
       void setupSkybox();
       unsigned int loadCubemap(std::vector<std::string> faces);

};
#endif