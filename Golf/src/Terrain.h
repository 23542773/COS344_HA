#ifndef TERRAIN_H
#define TERRAIN_H

#include <vector>
#include <glm/glm.hpp>
#include <GL/glew.h>

struct Vertex {
    glm::vec3 position;
    glm::vec2 texCoords;
    glm::vec3 normal;
};

class Terrain {
public:
    Terrain(int width, int depth);
    ~Terrain();
    void draw();

private:
    unsigned int VAO, VBO, EBO;
    int indexCount;
    void setupTerrain(int width, int depth);
};

#endif