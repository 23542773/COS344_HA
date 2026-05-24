#ifndef MESH_H
#define MESH_H

#include <vector>
#include <GL/glew.h>
#include "utils/ObjectLoader.h"
#include "Common.h"

class Mesh {
public:
    Mesh(const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices);
    ~Mesh();
    void draw(glm::mat4 view, glm::mat4 projection, glm::mat4 model);

private:
    unsigned int VAO, VBO, EBO;
    unsigned int indexCount;
    GLuint shaderProgram;
    void setupMesh(const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices);
};

#endif