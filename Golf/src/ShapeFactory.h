#ifndef SHAPE_FACTORY_H
#define SHAPE_FACTORY_H
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <string>
#include <vector>
enum ColliderType { NONE, AABB, SPHERE };

struct Collider {
  ColliderType type;
  glm::vec3 center;
  glm::vec3 halfSize; // for AABB
  float radius;       // for sphere
};

struct SceneObject {
  GLuint VAO;
  GLuint VBO;

  GLuint textureID;
  GLenum drawMode;
  int vertexCount;

  glm::vec3 position;
  glm::vec3 rotation;
  glm::vec3 scale;

  glm::vec3 color;

  bool textured;
  bool transparent;
  Collider collider;
};
class ShapeFactory {
public:
  static SceneObject createCube(glm::vec3 position, glm::vec3 scale,
                                glm::vec3 rotation, glm::vec3 color,
                                const std::string &texturePath = "");

  static SceneObject createPlane(glm::vec3 position, glm::vec2 size,
                                 glm::vec3 rotation, glm::vec3 color,
                                 const std::string &texturePath = "");

  static SceneObject createRamp(glm::vec3 position, glm::vec3 scale,
                                glm::vec3 rotation, glm::vec3 color,
                                const std::string &texturePath = "");

  static SceneObject createCylinder(glm::vec3 position, float radius,
                                    float height, int segments,
                                    glm::vec3 rotation, glm::vec3 color,
                                    const std::string &texturePath = "");

  static GLuint loadTexture(const std::string &path);
  static void drawObject(SceneObject &object, GLuint shader, glm::mat4 view,
                         glm::mat4 projection);
};

#endif
