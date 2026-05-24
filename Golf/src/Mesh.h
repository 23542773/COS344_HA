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
    void draw();

private:
    unsigned int VAO, VBO, EBO;
    unsigned int indexCount;
    void setupMesh(const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices);
};

#endif