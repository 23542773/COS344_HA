#pragma once
#include <glm/glm.hpp>
#include <string>
#include <unordered_map>

struct Material {
  glm::vec3 Ka = glm::vec3(0.0f);
  glm::vec3 Kd = glm::vec3(1.0f);
  glm::vec3 Ks = glm::vec3(0.0f);
  float Ns = 1.0f;
};

class MaterialLibrary {
public:
  bool loadMTL(const std::string &path);

  const Material *get(const std::string &name) const;

private:
  std::unordered_map<std::string, Material> materials;
};
