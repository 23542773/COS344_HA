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
#include "LightingSystem.h"
#include "Mesh.h"
#include "ShapeFactory.h"
#include "Skybox.h"
#include "Terrain.h"
#include "shader.hpp"
#include "utils/ObjectLoader.h"

using namespace glm;
using namespace std;

// Global pointers
Camera *camera;
Terrain *terrain;
Mesh *windmill;
Skybox *skybox;
LightingSystem *lighting;

float lastX = 640.0f;
float lastY = 360.0f;
bool firstMouse = true;

// Forward declarations
void mouse_callback(GLFWwindow *window, double xpos, double ypos);
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);
void processInput(GLFWwindow *window, float deltaTime, bool &spotlightOn, bool &nightVisionOn, bool &orbitMode, bool &orbitTargetSet);
void setupSceneObjects(std::vector<SceneObject> &sceneObjects);

// Helper functions for course construction
void addBorder(std::vector<SceneObject> &scene, glm::vec3 pos, glm::vec3 scale, glm::vec3 rot);
void addHoleCup(std::vector<SceneObject> &scene, glm::vec3 pos);
void addHolePath(std::vector<SceneObject> &scene, glm::vec3 pos, glm::vec3 scale, glm::vec3 rot);
void addHolePathV(std::vector<SceneObject> &scene, glm::vec3 pos, glm::vec3 scale, glm::vec3 rot);
void addHolePathBare(std::vector<SceneObject> &scene, glm::vec3 pos, glm::vec3 scale, glm::vec3 rot);
void addOverCover(std::vector<SceneObject> &scene, glm::vec3 pos, float width, float depth);
void addTunnel(std::vector<SceneObject> &scene, glm::vec3 pos, float width, float length);

const char *getError() {
  const char *errorDescription;
  glfwGetError(&errorDescription);
  if (errorDescription == nullptr)
    return "Unknown GLFW error";
  return errorDescription;
}

inline void startUpGLFW() {
  glfwInitHint(GLFW_PLATFORM, GLFW_PLATFORM_X11);
  glewExperimental = true;
  if (!glfwInit()) {
    throw getError();
  }
}

inline void startUpGLEW() {
  glewExperimental = true;
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

  GLFWwindow *window = glfwCreateWindow(1280, 720, "Team's Golf Course", NULL, NULL);

  if (window == NULL) {
    cout << getError() << endl;
    glfwTerminate();
    throw "Failed to open GLFW window.\n";
  }

  glfwMakeContextCurrent(window);
  startUpGLEW();

  glfwSetCursorPosCallback(window, mouse_callback);
  glfwSetScrollCallback(window, scroll_callback);
  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LESS);
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

void processInput(GLFWwindow *window, float deltaTime, bool &spotlightOn, 
                  bool &nightVisionOn, bool &orbitMode, bool &orbitTargetSet) {
  // Camera movement
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

  // Speed control
  if (glfwGetKey(window, GLFW_KEY_EQUAL) == GLFW_PRESS)
    camera->MovementSpeed += 0.1f;
  if (glfwGetKey(window, GLFW_KEY_MINUS) == GLFW_PRESS)
    camera->MovementSpeed -= 0.1f;
  if (camera->MovementSpeed < 1.0f) camera->MovementSpeed = 1.0f;
  if (camera->MovementSpeed > 20.0f) camera->MovementSpeed = 20.0f;

  // FOV control
  if (glfwGetKey(window, GLFW_KEY_I) == GLFW_PRESS)
    camera->Zoom += 0.5f;
  if (glfwGetKey(window, GLFW_KEY_O) == GLFW_PRESS)
    camera->Zoom -= 0.5f;
  if (camera->Zoom < 20.0f) camera->Zoom = 20.0f;
  if (camera->Zoom > 90.0f) camera->Zoom = 90.0f;

  // Rotation
  if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
    camera->processMouseMovement(-1.0f, 0.0f);
  if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
    camera->processMouseMovement(1.0f, 0.0f);

  // Toggle spotlight with 'H' key
  static bool hPressed = false;
  if (glfwGetKey(window, GLFW_KEY_H) == GLFW_PRESS && !hPressed) {
    spotlightOn = !spotlightOn;
    hPressed = true;
    cout << "Spotlight: " << (spotlightOn ? "ON" : "OFF") << endl;
  }
  if (glfwGetKey(window, GLFW_KEY_H) == GLFW_RELEASE) {
    hPressed = false;
  }

  // Toggle night vision with 'N' key
  static bool nPressed = false;
  if (glfwGetKey(window, GLFW_KEY_N) == GLFW_PRESS && !nPressed) {
    nightVisionOn = !nightVisionOn;
    nPressed = true;
    cout << "Night Vision: " << (nightVisionOn ? "ON" : "OFF") << endl;
  }
  if (glfwGetKey(window, GLFW_KEY_N) == GLFW_RELEASE) {
    nPressed = false;
  }

  // Toggle orbit mode with Middle Mouse Button
  static bool mPressed = false;
  if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_MIDDLE) == GLFW_PRESS && !mPressed) {
    orbitMode = !orbitMode;
    camera->setOrbitMode(orbitMode);
    mPressed = true;
    cout << "Orbit Mode: " << (orbitMode ? "ON" : "OFF") << endl;
  }
  if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_MIDDLE) == GLFW_RELEASE) {
    mPressed = false;
  }

  // When entering orbit mode, set orbit target to course center
  if (orbitMode && !orbitTargetSet) {
    camera->setOrbitTarget(glm::vec3(0.0f, 5.0f, 0.0f));
    orbitTargetSet = true;
  }
  if (!orbitMode) {
    orbitTargetSet = false;
  }
}

// Helper function implementations
glm::vec3 borderColor(0.18f, 0.09f, 0.03f);
glm::vec3 turfColor(0.1f, 0.7f, 0.2f);
glm::vec3 blackColor(0.02f, 0.02f, 0.02f);
glm::vec3 waterColor(0.1f, 0.4f, 0.8f);
glm::vec3 woodColor(0.6f, 0.4f, 0.2f);
glm::vec3 greyColor(0.5f, 0.5f, 0.5f);

void addBorder(std::vector<SceneObject> &scene, glm::vec3 pos, glm::vec3 scale, glm::vec3 rot) {
  scene.push_back(ShapeFactory::createCube(pos, scale, rot, borderColor));
}

void addHoleCup(std::vector<SceneObject> &scene, glm::vec3 pos) {
  scene.push_back(ShapeFactory::createCylinder(pos, 0.22f, 0.12f, 32, glm::vec3(0, 0, 0), blackColor));
}

void addHolePath(std::vector<SceneObject> &scene, glm::vec3 pos, glm::vec3 scale, glm::vec3 rot) {
  scene.push_back(ShapeFactory::createCube(pos, scale, rot, turfColor));
}

void addHolePathV(std::vector<SceneObject> &scene, glm::vec3 pos, glm::vec3 scale, glm::vec3 rot) {
  scene.push_back(ShapeFactory::createCube(pos, scale, rot, turfColor));
}

void addHolePathBare(std::vector<SceneObject> &scene, glm::vec3 pos, glm::vec3 scale, glm::vec3 rot) {
  scene.push_back(ShapeFactory::createCube(pos, scale, rot, turfColor));
}

void addOverCover(std::vector<SceneObject> &scene, glm::vec3 pos, float width, float depth) {
  // Create a roof/cover over the path
  scene.push_back(ShapeFactory::createCube(
      glm::vec3(pos.x, pos.y + 0.8f, pos.z), 
      glm::vec3(width, 0.1f, depth), 
      glm::vec3(0, 0, 0), 
      woodColor));
  
  // Add support posts at corners
  float halfW = width / 2.0f;
  float halfD = depth / 2.0f;
  
  scene.push_back(ShapeFactory::createCube(
      glm::vec3(pos.x - halfW + 0.3f, pos.y + 0.4f, pos.z - halfD + 0.3f), 
      glm::vec3(0.2f, 0.8f, 0.2f), 
      glm::vec3(0, 0, 0), 
      woodColor));
  scene.push_back(ShapeFactory::createCube(
      glm::vec3(pos.x + halfW - 0.3f, pos.y + 0.4f, pos.z - halfD + 0.3f), 
      glm::vec3(0.2f, 0.8f, 0.2f), 
      glm::vec3(0, 0, 0), 
      woodColor));
  scene.push_back(ShapeFactory::createCube(
      glm::vec3(pos.x - halfW + 0.3f, pos.y + 0.4f, pos.z + halfD - 0.3f), 
      glm::vec3(0.2f, 0.8f, 0.2f), 
      glm::vec3(0, 0, 0), 
      woodColor));
  scene.push_back(ShapeFactory::createCube(
      glm::vec3(pos.x + halfW - 0.3f, pos.y + 0.4f, pos.z + halfD - 0.3f), 
      glm::vec3(0.2f, 0.8f, 0.2f), 
      glm::vec3(0, 0, 0), 
      woodColor));
}

void addTunnel(std::vector<SceneObject> &scene, glm::vec3 pos, float width, float length) {
  // Create a tunnel: two side walls and a top
  float halfW = width / 2.0f;
  float halfL = length / 2.0f;
  
  // Left wall
  scene.push_back(ShapeFactory::createCube(
      glm::vec3(pos.x - halfW - 0.2f, pos.y + 0.5f, pos.z), 
      glm::vec3(0.2f, 1.0f, length), 
      glm::vec3(0, 0, 0), 
      greyColor));
  
  // Right wall
  scene.push_back(ShapeFactory::createCube(
      glm::vec3(pos.x + halfW + 0.2f, pos.y + 0.5f, pos.z), 
      glm::vec3(0.2f, 1.0f, length), 
      glm::vec3(0, 0, 0), 
      greyColor));
  
  // Top
  scene.push_back(ShapeFactory::createCube(
      glm::vec3(pos.x, pos.y + 1.0f, pos.z), 
      glm::vec3(width + 0.4f, 0.2f, length), 
      glm::vec3(0, 0, 0), 
      greyColor));
}

void setupSceneObjects(std::vector<SceneObject> &sceneObjects) {
  // Your existing setupSceneObjects code here...
  // (Keeping it short - the full course setup from your original)
  
  // For now, add a simple ground plane to test
  sceneObjects.push_back(ShapeFactory::createPlane(
      glm::vec3(40.0f, -0.5f, 24.0f), 
      glm::vec2(80.0f, 48.0f), 
      glm::vec3(0, 0, 0), 
      turfColor));
}

int main() {
  GLFWwindow *window;
  try {
    window = setUp();
  } catch (const char *e) {
    cout << e << endl;
    return -1;
  }

  // Initialize systems
  camera = new Camera(glm::vec3(0.0f, 2.0f, 5.0f), -90.0f, 0.0f);
  lighting = new LightingSystem();

  float deltaTime = 0.0f;
  float lastFrame = 0.0f;

  // State variables
  bool spotlightOn = false;
  bool nightVisionOn = false;
  bool orbitMode = false;
  bool orbitTargetSet = false;

  // Load skybox
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

  // Load windmill model
  std::vector<Vertex> vertices;
  std::vector<unsigned int> indices;
  if (loadOBJ("assets/models/windmill.obj", vertices, indices)) {
    windmill = new Mesh(vertices, indices);
  } else {
    std::cout << "Failed to load windmill.obj" << std::endl;
  }

  // Setup shader and lighting
  GLuint objectShader = LoadShaders("object.vert", "object.frag");
  glUseProgram(objectShader);
  lighting->setupLights(objectShader);

  // Setup scene objects
  std::vector<SceneObject> sceneObjects;
  setupSceneObjects(sceneObjects);

  while (!glfwWindowShouldClose(window)) {
    float currentFrame = glfwGetTime();
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;

    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
      glfwSetWindowShouldClose(window, true);

    // Process input
    processInput(window, deltaTime, spotlightOn, nightVisionOn, orbitMode, orbitTargetSet);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Update lighting
    glUseProgram(objectShader);
    lighting->updateLights(objectShader, camera->Position, camera->getFront(),
                           deltaTime, spotlightOn, nightVisionOn);

    // Get view and projection matrices
    glm::mat4 view = camera->getViewMatrix();
    glm::mat4 projection = camera->getProjectionMatrix(1280.0f / 720.0f);

    // Draw scene objects
    for (SceneObject &object : sceneObjects) {
      ShapeFactory::drawObject(object, objectShader, view, projection);
    }

    // Draw terrain (pass the shader, view, and projection matrices)
    terrain->draw(objectShader, view, projection);

    // Draw windmill (pass view, projection, and model matrices)
    if (windmill != nullptr) {
      glm::mat4 model = glm::mat4(1.0f);
      model = glm::translate(model, glm::vec3(-5.0f, 0.0f, -3.0f));
      model = glm::scale(model, glm::vec3(0.5f));
      windmill->draw(view, projection, model);
    }

    // Draw skybox (last, with depth testing disabled for background)
    glm::mat4 skyboxView = glm::mat4(glm::mat3(view));
    skybox->draw(skyboxView, projection, lighting->isNightTime());

    // Check OpenGL errors
    GLenum err;
    while ((err = glGetError()) != GL_NO_ERROR) {
      cout << "OpenGL Error: " << err << endl;
    }

    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  // Cleanup
  delete camera;
  delete lighting;
  delete skybox;
  delete terrain;
  delete windmill;

  glfwTerminate();
  return 0;
}