#ifndef TERRAIN_H
#define TERRAIN_H

#include <vector>
#include <glm/glm.hpp>
#include <GL/glew.h>
#include "Common.h"

class Terrain {
public:
    Terrain(int width, int depth);
    ~Terrain();
    void draw(GLuint shaderProgram, glm::mat4 view, glm::mat4 projection);

private:
    unsigned int VAO, VBO, EBO;
    int indexCount;
    void setupTerrain(int width, int depth);
};

#endif