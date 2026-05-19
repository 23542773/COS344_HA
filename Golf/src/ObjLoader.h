#ifndef OBJ_LOADER_H
#define OBJ_LOADER_H

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <string>
#include <vector>
#include <iostream>

#include "tiny_obj_loader.h"

namespace Geom {

    // Flyweight Shared State (Intrinsic State stored exactly once in VRAM)
    class ObjMesh {
    private:
        GLuint vao;
        GLuint vbo_pos;
        GLuint vbo_norm;
        GLuint vbo_uv;
        GLsizei vertexCount;

    public:
        ObjMesh() : vao(0), vbo_pos(0), vbo_norm(0), vbo_uv(0), vertexCount(0) {}

        ~ObjMesh() {
            if (vbo_pos)  glDeleteBuffers(1, &vbo_pos);
            if (vbo_norm) glDeleteBuffers(1, &vbo_norm);
            if (vbo_uv)   glDeleteBuffers(1, &vbo_uv);
            if (vao)      glDeleteVertexArrays(1, &vao);
        }

        bool loadFromFile(const std::string& filePath) {
            tinyobj::attrib_t attrib;
            std::vector<tinyobj::shape_t> shapes;
            std::vector<tinyobj::material_t> materials;
            std::string warn, err;

            // Load relative to your project plan's required models/ folder structure
            bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, filePath.c_str(), "models/");

            if (!warn.empty()) std::cout << "TinyOBJ Warning: " << warn << std::endl;
            if (!err.empty())  std::cerr << "TinyOBJ Error: " << err << std::endl;
            if (!ret) return false;

            std::vector<float> positions;
            std::vector<float> normals;
            std::vector<float> uvs;

            for (size_t s = 0; s < shapes.size(); s++) {
                size_t index_offset = 0;
                for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) {
                    size_t fv = size_t(shapes[s].mesh.num_face_vertices[f]);

                    for (size_t v = 0; v < fv; v++) {
                        tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];

                        // Attribute Binding 0: Positions
                        positions.push_back(attrib.vertices[3 * size_t(idx.vertex_index) + 0]);
                        positions.push_back(attrib.vertices[3 * size_t(idx.vertex_index) + 1]);
                        positions.push_back(attrib.vertices[3 * size_t(idx.vertex_index) + 2]);

                        // Attribute Binding 2: Normals
                        if (idx.normal_index >= 0) {
                            normals.push_back(attrib.normals[3 * size_t(idx.normal_index) + 0]);
                            normals.push_back(attrib.normals[3 * size_t(idx.normal_index) + 1]);
                            normals.push_back(attrib.normals[3 * size_t(idx.normal_index) + 2]);
                        } else {
                            normals.push_back(0.0f); normals.push_back(1.0f); normals.push_back(0.0f);
                        }

                        // Attribute Binding 3: Texture UV Coordinates
                        if (idx.texcoord_index >= 0) {
                            uvs.push_back(attrib.texcoords[2 * size_t(idx.texcoord_index) + 0]);
                            uvs.push_back(attrib.texcoords[2 * size_t(idx.texcoord_index) + 1]);
                        } else {
                            uvs.push_back(0.0f); uvs.push_back(0.0f);
                        }
                    }
                    index_offset += fv;
                }
            }

            vertexCount = static_cast<GLsizei>(positions.size() / 3);

            // Establish core hardware state configurations
            glGenVertexArrays(1, &vao);
            glBindVertexArray(vao);

            glGenBuffers(1, &vbo_pos);
            glBindBuffer(GL_ARRAY_BUFFER, vbo_pos);
            glBufferData(GL_ARRAY_BUFFER, positions.size() * sizeof(float), positions.data(), GL_STATIC_DRAW);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0); // location = 0
            glEnableVertexAttribArray(0);

            glGenBuffers(1, &vbo_norm);
            glBindBuffer(GL_ARRAY_BUFFER, vbo_norm);
            glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(float), normals.data(), GL_STATIC_DRAW);
            glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)0); // location = 2
            glEnableVertexAttribArray(1);

            glGenBuffers(1, &vbo_uv);
            glBindBuffer(GL_ARRAY_BUFFER, vbo_uv);
            glBufferData(GL_ARRAY_BUFFER, uvs.size() * sizeof(float), uvs.data(), GL_STATIC_DRAW);
            glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, (void*)0); // location = 3
            glEnableVertexAttribArray(2);

            glBindVertexArray(0);
            return true;
        }

        // Direct stream call execution 
        void bindAndDraw(bool wireframe) const {
            if (vao == 0 || vertexCount == 0) return;
            glBindVertexArray(vao);
            glDrawArrays(wireframe ? GL_LINES : GL_TRIANGLES, 0, vertexCount);
            glBindVertexArray(0);
        }
    };


    // Flyweight Context Instance (Extrinsic State sitting in RAM)
    struct MeshInstance {
        const ObjMesh* sharedGeometry; // Points to the single loaded asset
        glm::vec3 position;
        glm::vec3 rotation; // Euler angles: X, Y, Z in degrees
        glm::vec3 scale;

        MeshInstance(const ObjMesh* meshRef) 
            : sharedGeometry(meshRef), position(0.0f), rotation(0.0f), scale(1.0f) {}

        // Compute the affine transformation matrix on demand
        glm::mat4 getModelMatrix() const {
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, position);
            
            model = glm::rotate(model, glm::radians(rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
            model = glm::rotate(model, glm::radians(rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
            model = glm::rotate(model, glm::radians(rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
            
            model = glm::scale(model, scale);
            return model;
        }

        void render(GLuint modelMatrixLocation, bool wireframe) const {
            if (!sharedGeometry) return;
            
            // Pass the model matrix to the GPU vertex parallel pipeline
            glm::mat4 modelMat = getModelMatrix();
            glUniformMatrix4fv(modelMatrixLocation, 1, GL_FALSE, &modelMat[0][0]);
            
            // Draw the heavy mesh using zero state duplication
            sharedGeometry->bindAndDraw(wireframe);
        }
    };

} // namespace Geom

#endif // OBJ_LOADER_H