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

void setupSceneObjects(std::vector<SceneObject> &sceneObjects) {
  glm::vec3 turf(0.1f, 0.7f, 0.2f);
  glm::vec3 border(0.18f, 0.09f, 0.03f);
  glm::vec3 black(0.02f, 0.02f, 0.02f);
  glm::vec3 inclineRot(0.0f, 90.0f, 8.0f);

  // Hole 1
  sceneObjects.push_back(ShapeFactory::createCube(
      glm::vec3(33.0f, 1.4f, 16.5f), glm::vec3(6.0f, 0.3f, 3.0f),
      glm::vec3(0.0f, 0.0f, 0.0f), turf));

  // Top platform borders
  auto addBorder = [&](glm::vec3 pos, glm::vec3 scale) {
    sceneObjects.push_back(ShapeFactory::createCube(pos, scale, glm::vec3(0.0f), border));
  };

  addBorder(glm::vec3(33.0f, 1.7f, 15.0f), glm::vec3(6.0f, 0.7f, 0.4f));
  addBorder(glm::vec3(30.0f, 1.7f, 16.5f), glm::vec3(0.4f, 0.7f, 3.0f));
  addBorder(glm::vec3(36.0f, 1.7f, 16.5f), glm::vec3(0.4f, 0.7f, 3.0f));

  // Incline plane
  sceneObjects.push_back(ShapeFactory::createCube(glm::vec3(33.0f, 1.0f, 20.8f),
                                                  glm::vec3(6.0f, 0.3f, 4.0f),
                                                  inclineRot, turf));

  // Bottom flat platform
  sceneObjects.push_back(ShapeFactory::createCube(glm::vec3(33.0f, 0.7f, 24.5f),
                                                  glm::vec3(6.0f, 0.3f, 3.0f),
                                                  glm::vec3(0.0f), turf));

  // Bottom platform borders
  addBorder(glm::vec3(33.0f, 1.0f, 26.0f), glm::vec3(6.0f, 0.7f, 0.4f));
  addBorder(glm::vec3(30.0f, 1.0f, 24.5f), glm::vec3(0.4f, 0.7f, 3.0f));
  addBorder(glm::vec3(36.0f, 1.0f, 24.5f), glm::vec3(0.4f, 0.7f, 3.0f));

  // Hole cup
  sceneObjects.push_back(ShapeFactory::createCylinder(glm::vec3(33.0f, 0.9f, 24.5f),
                                                      0.22f, 0.12f, 32,
                                                      glm::vec3(0, 0, 0), black));

  // Hole 2 - U-shape
  sceneObjects.push_back(ShapeFactory::createCube(
      glm::vec3(-36.0f, 0.2f, 20.0f), glm::vec3(3.0f, 0.3f, 12.0f),
      glm::vec3(0.0f), turf));
  sceneObjects.push_back(ShapeFactory::createCube(
      glm::vec3(-30.0f, 0.2f, 24.5f), glm::vec3(15.0f, 0.3f, 3.0f),
      glm::vec3(0.0f), turf));
  sceneObjects.push_back(ShapeFactory::createCube(
      glm::vec3(-24.0f, 0.2f, 20.0f), glm::vec3(3.0f, 0.3f, 12.0f),
      glm::vec3(0.0f), turf));

  // Hole 2 borders
  addBorder(glm::vec3(-37.5f, 0.5f, 20.0f), glm::vec3(0.4f, 0.7f, 12.0f));
  addBorder(glm::vec3(-22.5f, 0.5f, 20.0f), glm::vec3(0.4f, 0.7f, 12.0f));
  addBorder(glm::vec3(-36.0f, 0.5f, 14.0f), glm::vec3(3.0f, 0.7f, 0.4f));
  addBorder(glm::vec3(-24.0f, 0.5f, 14.0f), glm::vec3(3.0f, 0.7f, 0.4f));
  addBorder(glm::vec3(-30.0f, 0.5f, 26.0f), glm::vec3(15.0f, 0.7f, 0.4f));
  addBorder(glm::vec3(-34.5f, 0.5f, 18.5f), glm::vec3(0.4f, 0.7f, 9.0f));
  addBorder(glm::vec3(-25.5f, 0.5f, 18.5f), glm::vec3(0.4f, 0.7f, 9.0f));
  addBorder(glm::vec3(-30.0f, 0.5f, 23.0f), glm::vec3(10.0f, 0.7f, 0.4f));

  // Hole 2 cup
  sceneObjects.push_back(ShapeFactory::createCylinder(glm::vec3(-24.0f, 0.3f, 16.0f),
                                                      0.22f, 0.12f, 32,
                                                      glm::vec3(0, 0, 0), black));
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

  // ═══════════════════════════════════════════════════════════════
  // TERRAIN: 79 wide (X) × 48 deep (Z)
  // Convention: low Z = top/north, high Z = bottom/south
  //             low X = left/west,  high X = right/east
  // Layout zones (matching reference):
  //   Left strip  x=1–16:  holes 16,2,3,4,5,6,18
  //   Center-left x=17–26: holes 14,15,10,17
  //   Center      x=27–35: holes 1,11
  //   Right-ctr   x=35–52: holes 7,8,12
  //   Right strip x=50–58: hole 9
  //   Far right   x=60–67: hole 13
  // ═══════════════════════════════════════════════════════════════

  // ─────────────────────────────────────────────────────────────
  // HOLE 16 — top-left, L-shape (vertical arm + dogleg right)
  // Tee at south, cup at east end
  // ─────────────────────────────────────────────────────────────
  addHolePathV   (sceneObjects, {5.0f, 0.2f,  4.5f}, {3.5f, 0.3f, 7.0f}, {0,0,0}); // vertical arm, open top+bot
  addHolePathBare(sceneObjects, {5.0f, 0.2f,  1.5f}, {3.5f, 0.3f, 1.5f}, {0,0,0}); // top junction filler
  addBorder(sceneObjects, {5.0f, 0.5f, 0.55f}, {3.5f+0.8f, 0.6f, 0.4f}, {0,0,0});  // north cap
  addHolePathH   (sceneObjects, {10.0f,0.2f,  1.5f}, {7.0f, 0.3f, 3.5f}, {0,0,0}); // horizontal arm east
  addBorder(sceneObjects, {6.75f,0.5f, 1.5f}, {0.4f, 0.6f, 3.5f}, {0,0,0});        // west side of horiz join
  addBorder(sceneObjects, {13.55f,0.5f,1.5f}, {0.4f, 0.6f, 3.5f}, {0,0,0});        // east cap
  addBorder(sceneObjects, {5.0f, 0.5f, 8.25f},{3.5f+0.8f,0.6f,0.4f},{0,0,0});       // south cap of vertical
  addHoleCup(sceneObjects, {13.0f, 0.35f, 1.5f});

  // ─────────────────────────────────────────────────────────────
  // HOLE 2 — left edge, large U-shape
  // Left arm, bottom bar, right arm — tee at south of left arm, cup at north of right arm
  // ─────────────────────────────────────────────────────────────
  // Left arm
  addHolePathV   (sceneObjects, {3.0f,  0.2f, 14.0f}, {3.5f, 0.3f, 14.0f}, {0,0,0});
  addBorder(sceneObjects, {3.0f, 0.5f, 20.8f}, {3.5f+0.8f, 0.6f, 0.4f}, {0,0,0}); // south cap
  // Bottom bar
  addHolePathBare(sceneObjects, {9.25f, 0.2f, 20.5f}, {9.0f, 0.3f, 3.5f}, {0,0,0});
  addBorder(sceneObjects, {9.25f,0.5f, 22.25f},{9.0f,       0.6f, 0.4f}, {0,0,0}); // south bar of connector
  // Right arm
  addHolePathV   (sceneObjects, {15.0f, 0.2f, 15.5f}, {3.5f, 0.3f, 11.0f}, {0,0,0});
  addBorder(sceneObjects, {15.0f,0.5f, 20.8f}, {3.5f+0.8f, 0.6f, 0.4f}, {0,0,0}); // south cap right arm
  // North caps
  addBorder(sceneObjects, {3.0f, 0.5f,  6.9f}, {3.5f+0.8f, 0.6f, 0.4f}, {0,0,0}); // north cap left arm
  addBorder(sceneObjects, {15.0f,0.5f,  9.9f}, {3.5f+0.8f, 0.6f, 0.4f}, {0,0,0}); // north cap right arm
  addHoleCup(sceneObjects, {15.0f, 0.35f, 10.5f});

  // ─────────────────────────────────────────────────────────────
  // HOLE 3 — left-center, wraps around the water feature
  // A narrow C-shape: top arm → east connector → south arm, water in the middle
  // ─────────────────────────────────────────────────────────────
  addHolePath(sceneObjects, {7.5f, 0.2f, 8.5f},  {3.5f, 0.3f, 3.5f}, {0,0,0}); // top segment (tee end, full borders)
  addHolePathBare(sceneObjects, {11.0f,0.2f, 9.5f},  {3.5f, 0.3f, 5.0f}, {0,0,0}); // east bridge (alongside water)
  addBorder(sceneObjects, {11.0f,0.5f, 6.9f},  {3.5f+0.8f,0.6f,0.4f},{0,0,0});   // north cap of bridge
  addBorder(sceneObjects, {11.0f,0.5f,12.05f}, {3.5f+0.8f,0.6f,0.4f},{0,0,0});   // south cap of bridge
  addBorder(sceneObjects, {9.15f,0.5f, 9.5f},  {0.4f,0.6f,5.0f},{0,0,0});        // west wall of bridge (faces water)
  addBorder(sceneObjects, {12.85f,0.5f,9.5f},  {0.4f,0.6f,5.0f},{0,0,0});        // east wall of bridge
  // WATER FEATURE (fills the pocket between the arms)
  sceneObjects.push_back(ShapeFactory::createCube({9.5f, 0.16f, 10.5f}, {4.5f, 0.12f, 5.5f}, {0,0,0}, water));
  addHoleCup(sceneObjects, {7.5f, 0.35f, 6.8f});

  // ─────────────────────────────────────────────────────────────
  // HOLE 4 — left side, vertical then dogleg right
  // ─────────────────────────────────────────────────────────────
  addHolePathV   (sceneObjects, {5.0f,  0.2f, 26.5f}, {3.5f, 0.3f, 9.0f}, {0,0,0});
  addBorder(sceneObjects, {5.0f, 0.5f, 22.0f}, {3.5f+0.8f,0.6f,0.4f},{0,0,0}); // north cap
  addBorder(sceneObjects, {5.0f, 0.5f, 31.0f}, {3.5f+0.8f,0.6f,0.4f},{0,0,0}); // south cap
  addHolePathBare(sceneObjects, {10.25f,0.2f, 31.0f}, {6.5f, 0.3f, 3.5f}, {0,0,0}); // dogleg east
  addBorder(sceneObjects, {10.25f,0.5f,29.25f},{6.5f,0.6f,0.4f},{0,0,0});      // north of dogleg
  addBorder(sceneObjects, {10.25f,0.5f,32.75f},{6.5f,0.6f,0.4f},{0,0,0});      // south of dogleg
  addBorder(sceneObjects, {13.65f,0.5f,31.0f},{0.4f,0.6f,3.5f},{0,0,0});       // east cap of dogleg
  addHoleCup(sceneObjects, {13.0f, 0.35f, 31.0f});

  // ─────────────────────────────────────────────────────────────
  // HOLE 5 — left side, straight vertical (tee south, cup north)
  // ─────────────────────────────────────────────────────────────
  addHolePath(sceneObjects, {5.0f, 0.2f, 37.5f}, {3.5f, 0.3f, 10.0f}, {0,0,0});
  addHoleCup (sceneObjects, {5.0f, 0.35f, 33.0f});

  // ─────────────────────────────────────────────────────────────
  // HOLE 18 — bottom-left, vertical south then dogleg east
  // ─────────────────────────────────────────────────────────────
  addHolePathV   (sceneObjects, {5.0f,  0.2f, 44.0f}, {3.5f, 0.3f, 7.0f}, {0,0,0});
  addBorder(sceneObjects, {5.0f, 0.5f, 40.55f},{3.5f+0.8f,0.6f,0.4f},{0,0,0}); // north cap
  addBorder(sceneObjects, {5.0f, 0.5f, 47.55f},{3.5f+0.8f,0.6f,0.4f},{0,0,0}); // south cap
  addHolePathBare(sceneObjects, {11.0f, 0.2f, 47.0f}, {8.0f, 0.3f, 3.5f}, {0,0,0}); // east dogleg
  addBorder(sceneObjects, {11.0f,0.5f, 45.25f},{8.0f,0.6f,0.4f},{0,0,0});      // north of dogleg
  addBorder(sceneObjects, {11.0f,0.5f, 48.75f},{8.0f,0.6f,0.4f},{0,0,0});      // south of dogleg
  addBorder(sceneObjects, {15.1f, 0.5f, 47.0f},{0.4f,0.6f,3.5f},{0,0,0});      // east cap
  addHoleCup(sceneObjects, {14.5f, 0.35f, 47.0f});

  // ─────────────────────────────────────────────────────────────
  // HOLE 6 — center-left, wide rectangle
  // ─────────────────────────────────────────────────────────────
  addHolePath(sceneObjects, {12.5f, 0.2f, 29.5f}, {6.0f, 0.3f, 9.0f}, {0,0,0});
  addHoleCup (sceneObjects, {12.5f, 0.35f, 25.5f});

  // ─────────────────────────────────────────────────────────────
  // HOLE 17 — center-bottom, wide horizontal block
  // ─────────────────────────────────────────────────────────────
  addHolePath(sceneObjects, {19.5f, 0.2f, 40.0f}, {10.0f, 0.3f, 6.0f}, {0,0,0});
  addHoleCup (sceneObjects, {16.0f, 0.35f, 40.0f});

  // ─────────────────────────────────────────────────────────────
  // HOLE 14 — top-center-left, long vertical with incline
  // Tee at south (high), ramps up, cup at north (elevated)
  // ─────────────────────────────────────────────────────────────
  // Lower flat section
  addHolePathV   (sceneObjects, {20.5f, 0.2f,  5.5f}, {4.0f, 0.3f, 7.0f}, {0,0,0});
  addBorder(sceneObjects, {20.5f,0.5f, 2.0f},  {4.0f+0.8f,0.6f,0.4f},{0,0,0}); // north cap lower
  addBorder(sceneObjects, {20.5f,0.5f, 9.0f},  {4.0f+0.8f,0.6f,0.4f},{0,0,0}); // south cap lower / transition
  // Incline
  sceneObjects.push_back(ShapeFactory::createCube({20.5f, 0.65f, 11.5f}, {4.0f,0.3f,4.5f}, {8.0f,0,0}, turf));
  addBorder(sceneObjects, {16.4f,0.95f,11.5f}, {0.4f,0.6f,4.5f},{0,0,0}); // left wall incline
  addBorder(sceneObjects, {24.6f,0.95f,11.5f}, {0.4f,0.6f,4.5f},{0,0,0}); // right wall incline
  // Upper flat section
  addHolePathV   (sceneObjects, {20.5f, 1.05f, 15.0f}, {4.0f, 0.3f, 3.5f}, {0,0,0});
  addBorder(sceneObjects, {20.5f,1.35f,13.25f},{4.0f+0.8f,0.6f,0.4f},{0,0,0}); // north cap upper
  addBorder(sceneObjects, {20.5f,1.35f,16.75f},{4.0f+0.8f,0.6f,0.4f},{0,0,0}); // south cap upper
  addHoleCup(sceneObjects, {20.5f, 1.2f, 14.0f});

  // ─────────────────────────────────────────────────────────────
  // HOLE 15 — center-left, vertical with over-cover on top half
  // ─────────────────────────────────────────────────────────────
  addHolePath(sceneObjects, {20.5f, 0.2f, 25.5f}, {4.0f, 0.3f, 10.0f}, {0,0,0});
  addOverCover(sceneObjects, {20.5f, 0.2f, 22.0f}, 4.0f, 5.5f);
  addHoleCup (sceneObjects, {20.5f, 0.35f, 30.0f});

  // ─────────────────────────────────────────────────────────────
  // HOLE 10 — center, vertical with over-cover on top half
  // ─────────────────────────────────────────────────────────────
  addHolePath(sceneObjects, {20.5f, 0.2f, 36.5f}, {4.0f, 0.3f, 9.0f}, {0,0,0});
  addOverCover(sceneObjects, {20.5f, 0.2f, 33.5f}, 4.0f, 4.5f);
  addHoleCup (sceneObjects, {20.5f, 0.35f, 40.5f});

  // ─────────────────────────────────────────────────────────────
  // HOLE 1 — top-center, elevated tee → incline → putting green
  // ─────────────────────────────────────────────────────────────
  // Elevated tee platform
  sceneObjects.push_back(ShapeFactory::createCube({30.5f, 1.5f, 2.5f}, {7.0f,0.35f,4.0f}, {0,0,0}, turf));
  addBorder(sceneObjects, {30.5f, 1.85f,  0.6f}, {7.8f, 0.7f, 0.4f}, {0,0,0}); // north wall
  addBorder(sceneObjects, {30.5f, 1.85f,  4.6f}, {7.8f, 0.7f, 0.4f}, {0,0,0}); // south wall (open side → incline)
  addBorder(sceneObjects, {26.9f, 1.85f,  2.5f}, {0.4f, 0.7f, 4.0f}, {0,0,0}); // west wall
  addBorder(sceneObjects, {34.1f, 1.85f,  2.5f}, {0.4f, 0.7f, 4.0f}, {0,0,0}); // east wall
  // Incline
  sceneObjects.push_back(ShapeFactory::createCube({30.5f, 1.05f, 8.5f}, {7.0f,0.3f,7.5f}, {8.0f,0,0}, turf));
  addBorder(sceneObjects, {26.9f, 1.2f, 8.5f}, {0.4f,0.5f,7.5f},{0,0,0}); // left wall of slope
  addBorder(sceneObjects, {34.1f, 1.2f, 8.5f}, {0.4f,0.5f,7.5f},{0,0,0}); // right wall of slope
  // Putting green (lower)
  sceneObjects.push_back(ShapeFactory::createCube({30.5f, 0.7f, 14.0f}, {7.0f,0.3f,5.0f}, {0,0,0}, turf));
  addBorder(sceneObjects, {30.5f, 1.05f, 16.6f},{7.8f,0.6f,0.4f},{0,0,0}); // south wall
  addBorder(sceneObjects, {26.9f, 1.05f, 14.0f},{0.4f,0.6f,5.0f},{0,0,0}); // west wall
  addBorder(sceneObjects, {34.1f, 1.05f, 14.0f},{0.4f,0.6f,5.0f},{0,0,0}); // east wall
  addHoleCup(sceneObjects, {30.5f, 0.85f, 14.0f});

  // ─────────────────────────────────────────────────────────────
  // HOLE 11 — bottom-center, tall vertical rect
  // ─────────────────────────────────────────────────────────────
  addHolePath(sceneObjects, {29.5f, 0.2f, 41.5f}, {5.5f, 0.3f, 11.0f}, {0,0,0});
  addHoleCup (sceneObjects, {29.5f, 0.35f, 36.5f});

  // ─────────────────────────────────────────────────────────────
  // HOLE 12 — bottom-center-right, wide horizontal
  // ─────────────────────────────────────────────────────────────
  addHolePath(sceneObjects, {38.5f, 0.2f, 44.5f}, {10.0f, 0.3f, 5.5f}, {0,0,0});
  addHoleCup (sceneObjects, {34.5f, 0.35f, 44.5f});

  // ─────────────────────────────────────────────────────────────
  // HOLE 7 — top-right-center, large square, FULLY covered
  // ─────────────────────────────────────────────────────────────
  addHolePath(sceneObjects, {38.5f, 0.2f, 8.0f}, {8.0f, 0.3f, 14.0f}, {0,0,0});
  addOverCover(sceneObjects, {38.5f, 0.2f, 8.0f}, 8.0f, 14.0f);
  addHoleCup (sceneObjects, {38.5f, 0.35f, 13.5f});

  // ─────────────────────────────────────────────────────────────
  // HOLE 8 — right of 7, taller rect, FULLY covered
  // ─────────────────────────────────────────────────────────────
  addHolePath(sceneObjects, {47.5f, 0.2f, 16.5f}, {8.0f, 0.3f, 19.0f}, {0,0,0});
  addOverCover(sceneObjects, {47.5f, 0.2f, 16.5f}, 8.0f, 19.0f);
  addHoleCup (sceneObjects, {47.5f, 0.35f, 24.5f});

  // ─────────────────────────────────────────────────────────────
  // HOLE 9 — right strip, very long vertical, tunnel at south end
  // ─────────────────────────────────────────────────────────────
  addHolePathV   (sceneObjects, {55.0f, 0.2f, 26.0f}, {5.0f, 0.3f, 34.0f}, {0,0,0});
  addBorder(sceneObjects, {55.0f,0.5f,  9.0f},{5.0f+0.8f,0.6f,0.4f},{0,0,0}); // north cap
  addBorder(sceneObjects, {55.0f,0.5f, 43.1f},{5.0f+0.8f,0.6f,0.4f},{0,0,0}); // south cap
  // Tunnel — bottom third of the path
  addTunnel(sceneObjects, {55.0f, 0.2f, 38.5f}, 5.0f, 9.0f);
  addHoleCup(sceneObjects, {55.0f, 0.35f, 11.0f});

  // ─────────────────────────────────────────────────────────────
  // HOLE 13 — far right, extremely long straight
  // ─────────────────────────────────────────────────────────────
  addHolePath(sceneObjects, {64.0f, 0.2f, 25.0f}, {4.5f, 0.3f, 44.0f}, {0,0,0});
  addHoleCup (sceneObjects, {64.0f, 0.35f, 5.0f});


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

    skybox->draw(skyboxView, projection, isNight);

    terrain->draw(view, projection);

    // Draw windmill
    if (windmill != nullptr) {
      glm::mat4 model = glm::mat4(1.0f);
      model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
      glUniformMatrix4fv(glGetUniformLocation(objectShader, "model"), 1, GL_FALSE, &model[0][0]);
      windmill->draw();
    }

    // Draw skybox
    glm::mat4 skyboxView = glm::mat4(glm::mat3(view));
    skybox->draw(skyboxView, projection, lighting->isNightTime());

    // Check OpenGL errors
    GLenum err;
    while ((err = glGetError()) != GL_NO_ERROR) {
      cout << "OpenGL Error: " << err << endl;
    }

    if (windmill != nullptr) {
    glm::mat4 model = glm::mat4(1.0f); 
    windmill->draw(view, projection, model);
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