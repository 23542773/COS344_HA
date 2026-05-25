#include "Material.h"
#include <fstream>
#include <iostream>
#include <sstream>

bool MaterialLibrary::loadMTL(const std::string &path) {
  std::ifstream file(path);
  if (!file.is_open()) {
    std::cerr << "Failed to open MTL: " << path << std::endl;
    return false;
  }

  std::string line;
  std::string currentName;

  while (std::getline(file, line)) {
    std::stringstream ss(line);

    std::string prefix;
    ss >> prefix;

    if (prefix == "newmtl") {
      ss >> currentName;
      materials[currentName] = Material();
    } else if (prefix == "Kd") {
      glm::vec3 kd;
      ss >> kd.r >> kd.g >> kd.b;
      materials[currentName].Kd = kd;
    } else if (prefix == "Ka") {
      glm::vec3 ka;
      ss >> ka.r >> ka.g >> ka.b;
      materials[currentName].Ka = ka;
    } else if (prefix == "Ks") {
      glm::vec3 ks;
      ss >> ks.r >> ks.g >> ks.b;
      materials[currentName].Ks = ks;
    } else if (prefix == "Ns") {
      ss >> materials[currentName].Ns;
    }
  }

  std::cout << "Loaded MTL: " << path << std::endl;
  return true;
}

const Material *MaterialLibrary::get(const std::string &name) const {
  auto it = materials.find(name);
  if (it == materials.end())
    return nullptr;
  return &it->second;
}
