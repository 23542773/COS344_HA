#ifndef TERRAIN_H
#define TERRAIN_H

#include <vector>
#include <glm/glm.hpp>
#include <GL/glew.h>
#include "Common.h"
#include "shader.hpp"

class Terrain {
public:
    Terrain(int width, int depth);
    ~Terrain();
    void draw(glm::mat4 view, glm::mat4 projection);

private:
    unsigned int VAO, VBO, EBO;
    int indexCount;
    GLuint shaderProgram;
    void setupTerrain(int width, int depth);
};

#endif