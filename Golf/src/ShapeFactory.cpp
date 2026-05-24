#include "ShapeFactory.h"

#include <cmath>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

#include "stb_image.h"

namespace {

void setupObject(SceneObject &object, std::vector<float> &vertices,
                 GLenum drawMode) {

  object.drawMode = drawMode;
  object.vertexCount = vertices.size() / 8;

  glGenVertexArrays(1, &object.VAO);
  glGenBuffers(1, &object.VBO);

  glBindVertexArray(object.VAO);

  glBindBuffer(GL_ARRAY_BUFFER, object.VBO);
  glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float),
               vertices.data(), GL_STATIC_DRAW);

  // Position
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)0);
  glEnableVertexAttribArray(0);

  // Color
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float),
                        (void *)(3 * sizeof(float)));
  glEnableVertexAttribArray(1);

  // Texture coordinates
  glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float),
                        (void *)(6 * sizeof(float)));
  glEnableVertexAttribArray(2);

  glBindVertexArray(0);
}

} // namespace

GLuint ShapeFactory::loadTexture(const std::string &path) {

  GLuint textureID;
  glGenTextures(1, &textureID);

  int width, height, channels;

  stbi_set_flip_vertically_on_load(true);

  unsigned char *data = stbi_load(path.c_str(), &width, &height, &channels, 0);

  if (!data) {
    std::cout << "Failed to load texture: " << path << std::endl;
    return 0;
  }

  GLenum format = GL_RGB;

  if (channels == 1)
    format = GL_RED;
  else if (channels == 3)
    format = GL_RGB;
  else if (channels == 4)
    format = GL_RGBA;

  glBindTexture(GL_TEXTURE_2D, textureID);

  glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format,
               GL_UNSIGNED_BYTE, data);

  glGenerateMipmap(GL_TEXTURE_2D);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                  GL_LINEAR_MIPMAP_LINEAR);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  stbi_image_free(data);

  return textureID;
}

SceneObject ShapeFactory::createCube(glm::vec3 position, glm::vec3 scale,
                                     glm::vec3 rotation, glm::vec3 color,
                                     const std::string &texturePath) {

  SceneObject object;

  object.position = position;
  object.rotation = rotation;
  object.scale = scale;
  object.color = color;

  object.textured = !texturePath.empty();
  object.transparent = false;

  if (object.textured)
    object.textureID = loadTexture(texturePath);
  else
    object.textureID = 0;

  float r = color.r;
  float g = color.g;
  float b = color.b;

  std::vector<float> vertices = {

      // positions           // colors     // tex

      // Front
      -0.5f, -0.5f, 0.5f, r, g, b, 0.0f, 0.0f, 0.5f, -0.5f, 0.5f, r, g, b, 1.0f,
      0.0f, 0.5f, 0.5f, 0.5f, r, g, b, 1.0f, 1.0f,

      0.5f, 0.5f, 0.5f, r, g, b, 1.0f, 1.0f, -0.5f, 0.5f, 0.5f, r, g, b, 0.0f,
      1.0f, -0.5f, -0.5f, 0.5f, r, g, b, 0.0f, 0.0f,

      // Back
      -0.5f, -0.5f, -0.5f, r, g, b, 0.0f, 0.0f, -0.5f, 0.5f, -0.5f, r, g, b,
      0.0f, 1.0f, 0.5f, 0.5f, -0.5f, r, g, b, 1.0f, 1.0f,

      0.5f, 0.5f, -0.5f, r, g, b, 1.0f, 1.0f, 0.5f, -0.5f, -0.5f, r, g, b, 1.0f,
      0.0f, -0.5f, -0.5f, -0.5f, r, g, b, 0.0f, 0.0f,

      // Left
      -0.5f, 0.5f, 0.5f, r, g, b, 1.0f, 0.0f, -0.5f, 0.5f, -0.5f, r, g, b, 1.0f,
      1.0f, -0.5f, -0.5f, -0.5f, r, g, b, 0.0f, 1.0f,

      -0.5f, -0.5f, -0.5f, r, g, b, 0.0f, 1.0f, -0.5f, -0.5f, 0.5f, r, g, b,
      0.0f, 0.0f, -0.5f, 0.5f, 0.5f, r, g, b, 1.0f, 0.0f,

      // Right
      0.5f, 0.5f, 0.5f, r, g, b, 1.0f, 0.0f, 0.5f, 0.5f, -0.5f, r, g, b, 1.0f,
      1.0f, 0.5f, -0.5f, -0.5f, r, g, b, 0.0f, 1.0f,

      0.5f, -0.5f, -0.5f, r, g, b, 0.0f, 1.0f, 0.5f, -0.5f, 0.5f, r, g, b, 0.0f,
      0.0f, 0.5f, 0.5f, 0.5f, r, g, b, 1.0f, 0.0f,

      // Top
      -0.5f, 0.5f, -0.5f, r, g, b, 0.0f, 1.0f, -0.5f, 0.5f, 0.5f, r, g, b, 0.0f,
      0.0f, 0.5f, 0.5f, 0.5f, r, g, b, 1.0f, 0.0f,

      0.5f, 0.5f, 0.5f, r, g, b, 1.0f, 0.0f, 0.5f, 0.5f, -0.5f, r, g, b, 1.0f,
      1.0f, -0.5f, 0.5f, -0.5f, r, g, b, 0.0f, 1.0f,

      // Bottom
      -0.5f, -0.5f, -0.5f, r, g, b, 0.0f, 1.0f, -0.5f, -0.5f, 0.5f, r, g, b,
      0.0f, 0.0f, 0.5f, -0.5f, 0.5f, r, g, b, 1.0f, 0.0f,

      0.5f, -0.5f, 0.5f, r, g, b, 1.0f, 0.0f, 0.5f, -0.5f, -0.5f, r, g, b, 1.0f,
      1.0f, -0.5f, -0.5f, -0.5f, r, g, b, 0.0f, 1.0f};

  setupObject(object, vertices, GL_TRIANGLES);

  return object;
}

SceneObject ShapeFactory::createPlane(glm::vec3 position, glm::vec2 size,
                                      glm::vec3 rotation, glm::vec3 color,
                                      const std::string &texturePath) {

  SceneObject object;

  object.position = position;
  object.rotation = rotation;
  object.scale = glm::vec3(size.x, 1.0f, size.y);
  object.color = color;

  object.textured = !texturePath.empty();
  object.transparent = false;

  if (object.textured)
    object.textureID = loadTexture(texturePath);
  else
    object.textureID = 0;

  float r = color.r;
  float g = color.g;
  float b = color.b;

  std::vector<float> vertices = {

      -0.5f, 0.0f, -0.5f, r, g, b, 0.0f, 0.0f,
      0.5f,  0.0f, -0.5f, r, g, b, 1.0f, 0.0f,
      0.5f,  0.0f, 0.5f,  r, g, b, 1.0f, 1.0f,

      0.5f,  0.0f, 0.5f,  r, g, b, 1.0f, 1.0f,
      -0.5f, 0.0f, 0.5f,  r, g, b, 0.0f, 1.0f,
      -0.5f, 0.0f, -0.5f, r, g, b, 0.0f, 0.0f};

  setupObject(object, vertices, GL_TRIANGLES);

  return object;
}

SceneObject ShapeFactory::createRamp(glm::vec3 position, glm::vec3 scale,
                                     glm::vec3 rotation, glm::vec3 color,
                                     const std::string &texturePath) {

  SceneObject ramp = createCube(position, scale, rotation, color, texturePath);

  return ramp;
}

SceneObject ShapeFactory::createCylinder(glm::vec3 position, float radius,
                                         float height, int segments,
                                         glm::vec3 rotation, glm::vec3 color,
                                         const std::string &texturePath) {

  SceneObject object;

  object.position = position;
  object.rotation = rotation;
  object.scale = glm::vec3(1.0f);
  object.color = color;

  object.textured = !texturePath.empty();
  object.transparent = false;

  if (object.textured)
    object.textureID = loadTexture(texturePath);
  else
    object.textureID = 0;

  float r = color.r;
  float g = color.g;
  float b = color.b;

  std::vector<float> vertices;

  for (int i = 0; i < segments; i++) {

    float theta1 = (float)i / segments * 2.0f * 3.14159f;
    float theta2 = (float)(i + 1) / segments * 2.0f * 3.14159f;

    float x1 = radius * cos(theta1);
    float z1 = radius * sin(theta1);

    float x2 = radius * cos(theta2);
    float z2 = radius * sin(theta2);

    vertices.insert(vertices.end(), {

                                        x1, 0.0f,   z1, r, g, b, 0.0f, 0.0f,
                                        x2, 0.0f,   z2, r, g, b, 1.0f, 0.0f,
                                        x2, height, z2, r, g, b, 1.0f, 1.0f,

                                        x2, height, z2, r, g, b, 1.0f, 1.0f,
                                        x1, height, z1, r, g, b, 0.0f, 1.0f,
                                        x1, 0.0f,   z1, r, g, b, 0.0f, 0.0f});
  }

  setupObject(object, vertices, GL_TRIANGLES);

  return object;
}

void ShapeFactory::drawObject(SceneObject &object, GLuint shader,
                              glm::mat4 view, glm::mat4 projection) {

  glm::mat4 model = glm::mat4(1.0f);

  model = glm::translate(model, object.position);

  model = glm::rotate(model, glm::radians(object.rotation.x),
                      glm::vec3(1.0f, 0.0f, 0.0f));

  model = glm::rotate(model, glm::radians(object.rotation.y),
                      glm::vec3(0.0f, 1.0f, 0.0f));

  model = glm::rotate(model, glm::radians(object.rotation.z),
                      glm::vec3(0.0f, 0.0f, 1.0f));

  model = glm::scale(model, object.scale);

  glUniformMatrix4fv(glGetUniformLocation(shader, "model"), 1, GL_FALSE,
                     &model[0][0]);

  glUniformMatrix4fv(glGetUniformLocation(shader, "view"), 1, GL_FALSE,
                     &view[0][0]);

  glUniformMatrix4fv(glGetUniformLocation(shader, "projection"), 1, GL_FALSE,
                     &projection[0][0]);

  glUniform1i(glGetUniformLocation(shader, "useTexture"), object.textured);

  if (object.textured) {
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, object.textureID);
    glUniform1i(glGetUniformLocation(shader, "texture1"), 0);
  }

  glBindVertexArray(object.VAO);

  glDrawArrays(object.drawMode, 0, object.vertexCount);

  glBindVertexArray(0);
}
