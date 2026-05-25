#ifndef MESH_H
#define MESH_H

#include <vector>
#include <GL/glew.h>
#include "utils/ObjectLoader.h"
#include "Common.h"

class Mesh {
public:
    unsigned int VAO;
    unsigned int indexCount;

    Mesh(const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices, GLuint shaderID);
    ~Mesh();
    void draw(glm::mat4 view, glm::mat4 projection, glm::mat4 model);

private:
    unsigned int VBO, EBO;
    GLuint shaderProgram;
    void setupMesh(const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices);
};

#endif