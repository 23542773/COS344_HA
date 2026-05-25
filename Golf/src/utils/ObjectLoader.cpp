#include "ObjectLoader.h"
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>

struct Material {
  glm::vec3 diffuseColor = glm::vec3(1.0f);
};

static std::map<std::string, Material> g_materials;

static bool loadMTL(const std::string &mtlPath) {
  std::ifstream file(mtlPath);

  if (!file.is_open()) {
    std::cerr << "Could not open MTL file: " << mtlPath << std::endl;
    return false;
  }

  std::string line;
  std::string currentMaterial;

  while (std::getline(file, line)) {
    std::stringstream ss(line);

    std::string prefix;
    ss >> prefix;

    if (prefix == "newmtl") {
      ss >> currentMaterial;
      g_materials[currentMaterial] = Material();
    } else if (prefix == "Kd") {
      float r, g, b;
      ss >> r >> g >> b;

      if (!currentMaterial.empty()) {
        g_materials[currentMaterial].diffuseColor = glm::vec3(r, g, b);
      }
    }
  }

  return true;
}

bool loadOBJ(const char *path, std::vector<Vertex> &outVertices,
             std::vector<unsigned int> &outIndices) {

  std::vector<glm::vec3> temp_positions;
  std::vector<glm::vec2> temp_uvs;
  std::vector<glm::vec3> temp_normals;

  std::ifstream file(path);

  if (!file.is_open()) {
    std::cerr << "Could not open " << path << std::endl;
    return false;
  }

  std::string directory = std::string(path);
  size_t slashPos = directory.find_last_of("/\\");

  if (slashPos != std::string::npos) {
    directory = directory.substr(0, slashPos + 1);
  } else {
    directory = "";
  }

  glm::vec3 currentColor(0.0f, 0.0f, 0.0f);

  std::string line;

  while (std::getline(file, line)) {
    std::stringstream ss(line);

    std::string prefix;
    ss >> prefix;

    if (prefix == "mtllib") {
      std::string mtlFile;
      ss >> mtlFile;

      loadMTL(directory + mtlFile);
    } else if (prefix == "usemtl") {
      std::string materialName;
      ss >> materialName;

      if (g_materials.count(materialName)) {
        currentColor = g_materials[materialName].diffuseColor;
      }
    } else if (prefix == "v") {
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
          if (segment.empty()) {
            indices.push_back(0);
          } else {
            indices.push_back(std::stoi(segment));
          }
        }

        Vertex v;

        v.position = temp_positions[indices[0] - 1];

        if (indices.size() > 1 && indices[1] > 0) {
          v.texCoords = temp_uvs[indices[1] - 1];
        } else {
          v.texCoords = glm::vec2(0.0f);
        }

        if (indices.size() > 2 && indices[2] > 0) {
          v.normal = temp_normals[indices[2] - 1];
        } else {
          v.normal = glm::vec3(0.0f, 1.0f, 0.0f);
        }

        v.color = currentColor;

        outVertices.push_back(v);
        outIndices.push_back(outVertices.size() - 1);
      }
    }
  }

  return true;
}
