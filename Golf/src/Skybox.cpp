#include "Skybox.h"
#include "shader.hpp"
#include "stb_image.h"
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

float skyboxVertices[] = {
    -1.0f, 1.0f,  -1.0f, -1.0f, -1.0f, -1.0f, 1.0f,  -1.0f, -1.0f,
    1.0f,  -1.0f, -1.0f, 1.0f,  1.0f,  -1.0f, -1.0f, 1.0f,  -1.0f,

    -1.0f, -1.0f, 1.0f,  -1.0f, -1.0f, -1.0f, -1.0f, 1.0f,  -1.0f,
    -1.0f, 1.0f,  -1.0f, -1.0f, 1.0f,  1.0f,  -1.0f, -1.0f, 1.0f,

    1.0f,  -1.0f, -1.0f, 1.0f,  -1.0f, 1.0f,  1.0f,  1.0f,  1.0f,
    1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  -1.0f, 1.0f,  -1.0f, -1.0f,

    -1.0f, -1.0f, 1.0f,  -1.0f, 1.0f,  1.0f,  1.0f,  1.0f,  1.0f,
    1.0f,  1.0f,  1.0f,  1.0f,  -1.0f, 1.0f,  -1.0f, -1.0f, 1.0f,

    -1.0f, 1.0f,  -1.0f, 1.0f,  1.0f,  -1.0f, 1.0f,  1.0f,  1.0f,
    1.0f,  1.0f,  1.0f,  -1.0f, 1.0f,  1.0f,  -1.0f, 1.0f,  -1.0f,

    -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, 1.0f,  1.0f,  -1.0f, -1.0f,
    1.0f,  -1.0f, -1.0f, -1.0f, -1.0f, 1.0f,  1.0f,  -1.0f, 1.0f};

Skybox::Skybox(std::vector<std::string> dayFaces,
               std::vector<std::string> nightFaces) {
  shaderProgram = LoadShaders("skybox.vert", "skybox.frag");

  setupSkybox();

  dayCubemapTexture = loadCubemap(dayFaces);
  nightCubemapTexture = loadCubemap(nightFaces);
}

Skybox::~Skybox() {
  glDeleteVertexArrays(1, &skyboxVAO);
  glDeleteBuffers(1, &skyboxVBO);
}

void Skybox::setupSkybox() {
  glGenVertexArrays(1, &skyboxVAO);
  glGenBuffers(1, &skyboxVBO);

  glBindVertexArray(skyboxVAO);

  glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), skyboxVertices,
               GL_STATIC_DRAW);

  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);

  glBindVertexArray(0);
}

unsigned int Skybox::loadCubemap(std::vector<std::string> faces) {
  unsigned int textureID;
  glGenTextures(1, &textureID);
  glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

  int width, height, nrChannels;

  stbi_set_flip_vertically_on_load(false);

  for (unsigned int i = 0; i < faces.size(); i++) {
    int width, height, nrChannels;
    unsigned char *data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 4);
    
    if (!data) {
        std::cerr << "CRITICAL: FAILED TO LOAD: " << faces[i] << " - " << stbi_failure_reason() << std::endl;
        continue;
    }

    std::cout << "Loading " << faces[i] << ": " << width << "x" << height << std::endl;

    // Use glTexImage2D, but CHECK for errors immediately
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGBA, width, height,
                 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    
    // Check for errors immediately after the binding
    GLenum err = glGetError();
    if (err != GL_NO_ERROR) {
        std::cerr << "GL error after glTexImage2D for face " << i << ": " << err << std::endl;
    }

    stbi_image_free(data);
}

  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

  return textureID;
}

void Skybox::draw(glm::mat4 view, glm::mat4 projection, bool isNight) {
  glDepthMask(GL_FALSE);
  glDepthFunc(GL_LEQUAL);

  glUseProgram(shaderProgram);

  GLint viewLoc = glGetUniformLocation(shaderProgram, "view");
    GLint projLoc = glGetUniformLocation(shaderProgram, "projection");

  glBindVertexArray(skyboxVAO);

  glm::mat4 viewNoTranslation = glm::mat4(glm::mat3(view));

  glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"), 1, GL_FALSE,
                     &viewNoTranslation[0][0]);

  glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1,
                     GL_FALSE, &projection[0][0]);

  glUniform1i(glGetUniformLocation(shaderProgram, "skybox"), 0);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_CUBE_MAP,
                isNight ? nightCubemapTexture : dayCubemapTexture);

  glDrawArrays(GL_TRIANGLES, 0, 36);

  glBindVertexArray(0);

  glDepthFunc(GL_LESS);
  glDepthMask(GL_TRUE);
}
