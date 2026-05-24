#ifndef OBJECT_LOADER_H
#define OBJECT_LOADER_H

#include <vector>
#include <string>
#include <glm/glm.hpp>
#include "../Common.h"

bool loadOBJ(const char* path, std::vector<Vertex>& outVertices, std::vector<unsigned int>& outIndices);

#endif