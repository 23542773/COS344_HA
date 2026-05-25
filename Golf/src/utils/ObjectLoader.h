#ifndef OBJECT_LOADER_H
#define OBJECT_LOADER_H

#include "../Common.h"
#include <glm/glm.hpp>
#include <string>
#include <vector>

bool loadOBJ(const char *path, std::vector<Vertex> &outVertices,
             std::vector<unsigned int> &outIndices);

#endif
