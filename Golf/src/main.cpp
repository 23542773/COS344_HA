#include <chrono>
#include <iostream>
#include <random>
#include <stdio.h>
#include <stdlib.h>
#include <thread>
#include <vector>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include "Camera.h"
#include "Mesh.h"
#include "ShapeFactory.h"
#include "Skybox.h"
#include "Terrain.h"
#include "shader.hpp"
#include "utils/ObjectLoader.h"

using namespace glm;
using namespace std;

Camera *camera;
Terrain *terrain;
Mesh *windmill;

float lastX = 640.0f;
float lastY = 360.0f;
bool firstMouse = true;

bool isNight = false;
Skybox *skybox;

// Forward declarations
void mouse_callback(GLFWwindow *window, double xpos, double ypos);
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);

const char *getError() {
  const char *errorDescription;
  glfwGetError(&errorDescription);

  if (errorDescription == nullptr)
    return "Unknown GLFW error";

  return errorDescription;
}

inline void startUpGLFW() {
  glfwInitHint(GLFW_PLATFORM, GLFW_PLATFORM_X11);

  glewExperimental = true; // Needed for core profile
  if (!glfwInit()) {
    throw getError();
  }
}

inline void startUpGLEW() {
  glewExperimental = true; // Needed in core profile

  GLenum err = glewInit();

  glGetError();

  if (err != GLEW_OK) {
    glfwTerminate();
    throw (const char *)glewGetErrorString(err);
  }
}

inline GLFWwindow *setUp() {
  startUpGLFW();

  glfwWindowHint(GLFW_SAMPLES, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  GLFWwindow *window;

  window = glfwCreateWindow(1280, 720, "Team's Golf Course", NULL, NULL);

  if (window == NULL) {
    cout << getError() << endl;

    glfwTerminate();

    throw "Failed to open GLFW window. If you have an Intel GPU, they are not "
          "3.3 compatible. Try the 2.1 version of the tutorials.\n";
  }

  glfwMakeContextCurrent(window);

  startUpGLEW();

  glfwSetCursorPosCallback(window, mouse_callback);
  glfwSetScrollCallback(window, scroll_callback);
  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LESS);

  // glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

  return window;
}

void mouse_callback(GLFWwindow *window, double xpos, double ypos) {
  if (firstMouse) {
    lastX = (float)xpos;
    lastY = (float)ypos;
    firstMouse = false;
  }

  float xoffset = (float)xpos - lastX;
  float yoffset = lastY - (float)ypos;

  lastX = (float)xpos;
  lastY = (float)ypos;

  camera->processMouseMovement(xoffset, yoffset);
}

void scroll_callback(GLFWwindow *window, double xoffset, double yoffset) {
  camera->processMouseScroll((float)yoffset);
}
glm::vec3 turf(0.1f, 0.75f, 0.2f);
glm::vec3 border(0.22f, 0.11f, 0.04f);
glm::vec3 concrete(0.4f, 0.4f, 0.4f);
glm::vec3 water(0.1f, 0.4f, 0.8f);
glm::vec3 black(0.02f, 0.02f, 0.02f);

void addBorder(std::vector<SceneObject> &scene, glm::vec3 pos, glm::vec3 scale,
               glm::vec3 rot) {

  scene.push_back(ShapeFactory::createCube(pos, scale, rot, border));
}

void addHoleCup(std::vector<SceneObject> &scene, glm::vec3 pos) {

  scene.push_back(ShapeFactory::createCylinder(pos,
                                               0.22f,              // radius
                                               0.12f,              // height
                                               32,                 // segments
                                               glm::vec3(0, 0, 0), // rotation
                                               black));
}

int main() {
  GLFWwindow *window;

  try {
    window = setUp();
  } catch (const char *e) {
    cout << e << endl;
    return -1;
  }

  camera = new Camera(glm::vec3(0.0f, 2.0f, 5.0f), -90.0f, 0.0f);

  float deltaTime = 0.0f;
  float lastFrame = 0.0f;

  std::vector<std::string> dayFaces = {
      "assets/skybox/day_right.png", "assets/skybox/day_left.png",
      "assets/skybox/day_top.png",   "assets/skybox/day_bottom.png",
      "assets/skybox/day_front.png", "assets/skybox/day_back.png"};

  std::vector<std::string> nightFaces = {
      "assets/skybox/night_right.png", "assets/skybox/night_left.png",
      "assets/skybox/night_top.png",   "assets/skybox/night_bottom.png",
      "assets/skybox/night_front.png", "assets/skybox/night_back.png"};

  skybox = new Skybox(dayFaces, nightFaces);
  terrain = new Terrain(79, 48);

  std::vector<Vertex> vertices;
  std::vector<unsigned int> indices;
  if (loadOBJ("assets/models/windmill.obj", vertices, indices)) {
    windmill = new Mesh(vertices, indices);
  } else {
    std::cout << "Failed to load windmill.obj" << std::endl;
  }

  GLuint objectShader = LoadShaders("object.vert", "object.frag");

  // any scene code goes here, just push objects onto the sceneObjects vector

  std::vector<SceneObject> sceneObjects;
  // hole 1
  glm::vec3 turf(0.1f, 0.7f, 0.2f);
  glm::vec3 border(0.18f, 0.09f, 0.03f);
  glm::vec3 holeColor(0.01f, 0.01f, 0.01f);

  glm::vec3 inclineRot(0.0f, 90.0f, 8.0f);

  // Top start platform
  sceneObjects.push_back(ShapeFactory::createCube(
      glm::vec3(33.0f, 1.4f, 16.5f), glm::vec3(6.0f, 0.3f, 3.0f),
      glm::vec3(0.0f, 0.0f, 0.0f), turf));

  // Top platform borders (NO back edge - connects to incline)
  addBorder(sceneObjects, glm::vec3(33.0f, 1.7f, 15.0f),
            glm::vec3(6.0f, 0.7f, 0.4f), glm::vec3(0.0f, 0.0f, 0.0f));

  addBorder(sceneObjects, glm::vec3(30.0f, 1.7f, 16.5f),
            glm::vec3(0.4f, 0.7f, 3.0f), glm::vec3(0.0f, 0.0f, 0.0f));

  addBorder(sceneObjects, glm::vec3(36.0f, 1.7f, 16.5f),
            glm::vec3(0.4f, 0.7f, 3.0f), glm::vec3(0.0f, 0.0f, 0.0f));

  // incline plane
  sceneObjects.push_back(ShapeFactory::createCube(glm::vec3(33.0f, 1.0f, 20.8f),
                                                  glm::vec3(6.0f, 0.3f, 4.0f),
                                                  inclineRot, turf));

  // Bottom flat platform
  sceneObjects.push_back(ShapeFactory::createCube(glm::vec3(33.0f, 0.7f, 24.5f),
                                                  glm::vec3(6.0f, 0.3f, 3.0f),
                                                  glm::vec3(0.0f), turf));

  // Bottom platform borders (NO top edge - connects to incline)
  addBorder(sceneObjects, glm::vec3(33.0f, 1.0f, 26.0f),
            glm::vec3(6.0f, 0.7f, 0.4f), glm::vec3(0.0f));

  addBorder(sceneObjects, glm::vec3(30.0f, 1.0f, 24.5f),
            glm::vec3(0.4f, 0.7f, 3.0f), glm::vec3(0.0f));

  addBorder(sceneObjects, glm::vec3(36.0f, 1.0f, 24.5f),
            glm::vec3(0.4f, 0.7f, 3.0f), glm::vec3(0.0f));

  addHoleCup(sceneObjects, glm::vec3(33.0f, 0.9f, 24.5f));
  // hole 2

  // Left (vertical bar of U)
  sceneObjects.push_back(ShapeFactory::createCube(
      glm::vec3(-36.0f, 0.2f, 20.0f), glm::vec3(3.0f, 0.3f, 12.0f),
      glm::vec3(0.0f), turf));

  // Bottom (middle of U)
  sceneObjects.push_back(ShapeFactory::createCube(
      glm::vec3(-30.0f, 0.2f, 24.5f), glm::vec3(15.0f, 0.3f, 3.0f),
      glm::vec3(0.0f), turf));

  // Right (vertical bar of U)
  sceneObjects.push_back(ShapeFactory::createCube(
      glm::vec3(-24.0f, 0.2f, 20.0f), glm::vec3(3.0f, 0.3f, 12.0f),
      glm::vec3(0.0f), turf));

  // borders
  // left outside
  addBorder(sceneObjects, glm::vec3(-37.5f, 0.5f, 20.0f),
            glm::vec3(0.4f, 0.7f, 12.0f), glm::vec3(0.0f));

  // right outside
  addBorder(sceneObjects, glm::vec3(-22.5f, 0.5f, 20.0f),
            glm::vec3(0.4f, 0.7f, 12.0f), glm::vec3(0.0f));

  // top left
  addBorder(sceneObjects, glm::vec3(-36.0f, 0.5f, 14.0f),
            glm::vec3(3.0f, 0.7f, 0.4f), glm::vec3(0.0f));

  // top right
  addBorder(sceneObjects, glm::vec3(-24.0f, 0.5f, 14.0f),
            glm::vec3(3.0f, 0.7f, 0.4f), glm::vec3(0.0f));

  // Bottom outer (closed U bottom)
  addBorder(sceneObjects, glm::vec3(-30.0f, 0.5f, 26.0f),
            glm::vec3(15.0f, 0.7f, 0.4f), glm::vec3(0.0f));

  // Inner borders

  // Left inner vert
  addBorder(sceneObjects, glm::vec3(-34.5f, 0.5f, 18.5f),
            glm::vec3(0.4f, 0.7f, 9.0f), glm::vec3(0.0f));

  // Right inner vert
  addBorder(sceneObjects, glm::vec3(-25.5f, 0.5f, 18.5f),
            glm::vec3(0.4f, 0.7f, 9.0f), glm::vec3(0.0f));

  // Bottom inner
  addBorder(sceneObjects, glm::vec3(-30.0f, 0.5f, 23.0f),
            glm::vec3(10.0f, 0.7f, 0.4f), glm::vec3(0.0f));

  // hole placement
  addHoleCup(sceneObjects, glm::vec3(-24.0f, 0.3f, 16.0f));
  // hole 3
  // hole 4
  // hole 5
  // hole 6
  // hole 7
  // hole 8
  // hole 9
  //hole 10
// ========== HOLE 10 ==========
// L-shaped path: straight ahead, then turns left up a ramp to an elevated cup.
// Difficulty: Easy.

// --- Lower start platform (tee area) ---
sceneObjects.push_back(ShapeFactory::createCube(
    glm::vec3(0.0f, 0.2f, 18.0f), glm::vec3(4.0f, 0.3f, 3.0f), glm::vec3(0.0f), turf));

addBorder(sceneObjects, glm::vec3(0.0f, 0.5f, 16.5f), glm::vec3(4.0f, 0.7f, 0.4f), glm::vec3(0.0f)); // back
addBorder(sceneObjects, glm::vec3(-2.2f, 0.5f, 18.0f), glm::vec3(0.4f, 0.7f, 3.0f), glm::vec3(0.0f)); // left
addBorder(sceneObjects, glm::vec3(2.2f, 0.5f, 18.0f), glm::vec3(0.4f, 0.7f, 3.0f), glm::vec3(0.0f)); // right

// --- Straight segment heading +Z ---
sceneObjects.push_back(ShapeFactory::createCube(
    glm::vec3(0.0f, 0.2f, 21.5f), glm::vec3(4.0f, 0.3f, 4.0f), glm::vec3(0.0f), turf));

addBorder(sceneObjects, glm::vec3(-2.2f, 0.5f, 21.5f), glm::vec3(0.4f, 0.7f, 4.0f), glm::vec3(0.0f)); // left wall
addBorder(sceneObjects, glm::vec3(2.2f, 0.5f, 21.5f), glm::vec3(0.4f, 0.7f, 4.0f), glm::vec3(0.0f)); // right wall

// --- Corner junction ---
sceneObjects.push_back(ShapeFactory::createCube(
    glm::vec3(-2.0f, 0.2f, 23.8f), glm::vec3(8.0f, 0.3f, 4.0f), glm::vec3(0.0f), turf));

addBorder(sceneObjects, glm::vec3(-2.0f, 0.5f, 25.9f), glm::vec3(8.0f, 0.7f, 0.4f), glm::vec3(0.0f)); // bottom wall
addBorder(sceneObjects, glm::vec3(2.2f, 0.5f, 23.8f), glm::vec3(0.4f, 0.7f, 4.0f), glm::vec3(0.0f)); // right wall cap

// --- Left-running arm heading -X ---
sceneObjects.push_back(ShapeFactory::createCube(
    glm::vec3(-8.5f, 0.2f, 23.8f), glm::vec3(5.0f, 0.3f, 4.0f), glm::vec3(0.0f), turf));

addBorder(sceneObjects, glm::vec3(-8.5f, 0.5f, 21.8f), glm::vec3(5.0f, 0.7f, 0.4f), glm::vec3(0.0f)); // top wall
addBorder(sceneObjects, glm::vec3(-8.5f, 0.5f, 25.9f), glm::vec3(5.0f, 0.7f, 0.4f), glm::vec3(0.0f)); // bottom wall

// --- Ramp: rises in -X direction ---
sceneObjects.push_back(ShapeFactory::createCube(
    glm::vec3(-13.2f, 0.42f, 23.8f), glm::vec3(3.0f, 0.2f, 4.0f), glm::vec3(0.0f, 0.0f, -12.0f), turf));

addBorder(sceneObjects, glm::vec3(-13.2f, 0.65f, 21.8f), glm::vec3(3.0f, 0.5f, 0.4f), glm::vec3(0.0f)); // top wall
addBorder(sceneObjects, glm::vec3(-13.2f, 0.65f, 25.9f), glm::vec3(3.0f, 0.5f, 0.4f), glm::vec3(0.0f)); // bottom wall

// --- Elevated putting tier ---
// Removed side borders so the ball can exit the ramp onto this platform freely.
sceneObjects.push_back(ShapeFactory::createCube(
    glm::vec3(-16.5f, 0.75f, 23.8f), glm::vec3(4.0f, 0.3f, 4.0f), glm::vec3(0.0f), turf));

addBorder(sceneObjects, glm::vec3(-16.5f, 1.05f, 21.8f), glm::vec3(4.0f, 0.7f, 0.4f), glm::vec3(0.0f)); // top
addBorder(sceneObjects, glm::vec3(-16.5f, 1.05f, 25.9f), glm::vec3(4.0f, 0.7f, 0.4f), glm::vec3(0.0f)); // bottom

// Hole cup
addHoleCup(sceneObjects, glm::vec3(-16.5f, 0.84f, 23.8f));
  // hole 11
  // hole 12
  // hole 13
  // hole 14
  // hole 15
  // hole 16
  // hole 17
  // hole 18
  while (!glfwWindowShouldClose(window)) {

    float currentFrame = glfwGetTime();
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;

    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
      glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
      camera->processKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
      camera->processKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
      camera->processKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
      camera->processKeyboard(RIGHT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
      camera->processKeyboard(UP, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
      camera->processKeyboard(DOWN, deltaTime);

    if (glfwGetKey(window, GLFW_KEY_EQUAL) == GLFW_PRESS)
      camera->MovementSpeed += 0.1f;

    if (glfwGetKey(window, GLFW_KEY_MINUS) == GLFW_PRESS)
      camera->MovementSpeed -= 0.1f;

    if (glfwGetKey(window, GLFW_KEY_I) == GLFW_PRESS)
      camera->Zoom += 0.5f;

    if (glfwGetKey(window, GLFW_KEY_O) == GLFW_PRESS)
      camera->Zoom -= 0.5f;

    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
      camera->processMouseMovement(-1.0f, 0.0f);

    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
      camera->processMouseMovement(1.0f, 0.0f);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glm::mat4 view = camera->getViewMatrix();
    glm::mat4 projection = camera->getProjectionMatrix(1280.0f / 720.0f);

    glUseProgram(objectShader);

    for (SceneObject &object : sceneObjects) {

      ShapeFactory::drawObject(object, objectShader, view, projection);
    }
    glm::mat4 skyboxView = glm::mat4(glm::mat3(view));

    skybox->draw(skyboxView, projection, isNight);

    terrain->draw();

    GLenum err;
    while ((err = glGetError()) != GL_NO_ERROR) {
      cout << "OpenGL Error: " << err << endl;
    }

    if (windmill != nullptr) {
      windmill->draw();
    }

    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  glfwTerminate();
  return 0;
}
