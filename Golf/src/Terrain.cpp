#include "Terrain.h"

Terrain::Terrain(int width, int depth) {
    setupTerrain(width, depth);
}

void Terrain::setupTerrain(int width, int depth) {
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;

    // 1. Create Grid Vertices
    for (int z = 0; z <= depth; ++z) {
        for (int x = 0; x <= width; ++x) {
            Vertex v;
            v.position = glm::vec3((float)x, 0.0f, (float)z);
            v.texCoords = glm::vec2((float)x / width, (float)z / depth);
            v.normal = glm::vec3(0.0f, 1.0f, 0.0f); // Default flat normal
            vertices.push_back(v);
        }
    }

    // 2. Create Grid Indices (Connecting the dots)
    for (int z = 0; z < depth; ++z) {
        for (int x = 0; x < width; ++x) {
            int row1 = z * (width + 1);
            int row2 = (z + 1) * (width + 1);

            // Triangle 1
            indices.push_back(row1 + x);
            indices.push_back(row2 + x);
            indices.push_back(row1 + x + 1);

            // Triangle 2
            indices.push_back(row1 + x + 1);
            indices.push_back(row2 + x);
            indices.push_back(row2 + x + 1);
        }
    }
    indexCount = indices.size();

    // 3. Upload to GPU (Standard VBO/EBO/VAO setup)
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    // Vertex Attributes
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texCoords));
    glEnableVertexAttribArray(2);
    glBindVertexArray(0);
}

void Terrain::draw() {
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}