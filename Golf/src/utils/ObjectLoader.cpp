#include "ObjectLoader.h"
#include <fstream>
#include <iostream>
#include <sstream>

bool loadOBJ(const char* path, std::vector<Vertex>& outVertices, std::vector<unsigned int>& outIndices) {
    std::vector<glm::vec3> temp_positions;
    std::vector<glm::vec2> temp_uvs;
    std::vector<glm::vec3> temp_normals;
    
    std::ifstream file(path);
    if (!file.is_open()) {
        std::cerr << "Could not open " << path << std::endl;
        return false;
    }

    std::string line;
    while (std::getline(file, line)) {
        std::stringstream ss(line);
        std::string prefix;
        ss >> prefix;

        if (prefix == "v") {
            glm::vec3 pos;
            ss >> pos.x >> pos.y >> pos.z;
            temp_positions.push_back(pos);
        } else if (prefix == "vt") {
            glm::vec2 uv;
            ss >> uv.x >> uv.y;
            temp_uvs.push_back(uv);
        } else if (prefix == "vn") {
            glm::vec3 norm;
            ss >> norm.x >> norm.y >> norm.z;
            temp_normals.push_back(norm);
        } else if (prefix == "f") {
            std::string vertexData;
            while (ss >> vertexData) {
                std::stringstream vss(vertexData);
                std::string segment;
                std::vector<int> indices;
                while (std::getline(vss, segment, '/')) {
                    if (segment.empty()) indices.push_back(0);
                    else indices.push_back(std::stoi(segment));
                }
                
                // i am assuming standard f v/vt/vn format
                Vertex v;
                v.position = temp_positions[indices[0] - 1];
                v.texCoords = temp_uvs[indices[1] - 1];
                v.normal = temp_normals[indices[2] - 1];
                outVertices.push_back(v);
                outIndices.push_back(outVertices.size() - 1);
            }
        }
    }
    return true;
}