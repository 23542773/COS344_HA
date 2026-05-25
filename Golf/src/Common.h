#ifndef COMMON_H
#define COMMON_H

#include <glm/glm.hpp>

struct Vertex {
  glm::vec3 position;
  glm::vec2 texCoords;
  glm::vec3 normal;
  glm::vec3 color;
};

#endif
