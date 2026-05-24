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

//-----------------------------------
// Helper to create a straight hole segment
Mesh* createStraightHole(glm::vec3 start, glm::vec3 end, float width) {
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;

    // Calculate direction and right vector for the path
    glm::vec3 dir = glm::normalize(end - start);
    glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
    glm::vec3 right = glm::normalize(glm::cross(dir, up)) * (width * 0.5f);

    // 1. The Green Putting Surface (y = 0.01f to sit slightly above your base terrain)
    float surfaceY = 0.1f;
    glm::vec3 p0 = start - right + glm::vec3(0, surfaceY, 0); // Bottom left
    glm::vec3 p1 = start + right + glm::vec3(0, surfaceY, 0); // Bottom right
    glm::vec3 p2 = end - right + glm::vec3(0, surfaceY, 0);   // Top left
    glm::vec3 p3 = end + right + glm::vec3(0, surfaceY, 0);   // Top right

    // Add surface vertices (Green)
    vertices.push_back({p0, glm::vec2(0,0), up}); // 0
    vertices.push_back({p1, glm::vec2(1,0), up}); // 1
    vertices.push_back({p2, glm::vec2(0,1), up}); // 2
    vertices.push_back({p3, glm::vec2(1,1), up}); // 3

    indices.insert(indices.end(), {0, 1, 2, 1, 3, 2});

    // --- You can add the raised brown borders here later ---
    // For now, let's just get the layout paths down!

    return new Mesh(vertices, indices);
}

//------------------------------------------------------------------------

// Helper to place a path segment and its borders
void addHoleSegment(std::vector<SceneObject> &scene, glm::vec3 pos, glm::vec3 scale, glm::vec3 rot) {
    // The putting surface
    scene.push_back(ShapeFactory::createCube(pos, scale, rot, turf));
    
    // Add border logic here based on scale (or just manually place borders for now)
}

void addHolePath(std::vector<SceneObject> &scene, glm::vec3 pos, glm::vec3 scale, glm::vec3 rot) {
    // Putting surface
    scene.push_back(ShapeFactory::createCube(pos, scale, rot, turf));
    
    // Left border
    addBorder(scene, pos + glm::vec3(-scale.x/2 - 0.2f, 0.3f, 0), glm::vec3(0.4f, 0.7f, scale.z), rot);
    // Right border
    addBorder(scene, pos + glm::vec3(scale.x/2 + 0.2f, 0.3f, 0), glm::vec3(0.4f, 0.7f, scale.z), rot);
}

void buildHole(std::vector<SceneObject> &scene, glm::vec3 pathPos, glm::vec3 pathScale, glm::vec3 cupPos) {
    addHolePath(scene, pathPos, pathScale, glm::vec3(0));
    addHoleCup(scene, cupPos);
}

int main() {
  GLFWwindow *window;

  try {
    window = setUp();
  } catch (const char *e) {
    cout << e << endl;
    return -1;
  }

  camera = new Camera(glm::vec3(40.0f, 20.0f, 40.0f), -90.0f, -45.0f);

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

  float offsetX = 5.0f; // Shift everything into the positive zone
  float offsetZ = 5.0f;

  // =====================
  // HOLE 1 - Elevated platform with incline down to green
  // =====================
  glm::vec3 inclineRot(0.0f, 90.0f, 8.0f);

  // Top start platform
  sceneObjects.push_back(ShapeFactory::createCube(
      glm::vec3(33.0f, 1.4f, 16.5f), glm::vec3(6.0f, 0.3f, 3.0f),
      glm::vec3(0.0f), turf));

  addBorder(sceneObjects, glm::vec3(33.0f, 1.7f, 15.0f), glm::vec3(6.0f, 0.7f, 0.4f), glm::vec3(0.0f));
  addBorder(sceneObjects, glm::vec3(30.0f, 1.7f, 16.5f), glm::vec3(0.4f, 0.7f, 3.0f), glm::vec3(0.0f));
  addBorder(sceneObjects, glm::vec3(36.0f, 1.7f, 16.5f), glm::vec3(0.4f, 0.7f, 3.0f), glm::vec3(0.0f));

  // Incline
  sceneObjects.push_back(ShapeFactory::createCube(
      glm::vec3(33.0f, 1.0f, 20.8f), glm::vec3(6.0f, 0.3f, 4.0f), inclineRot, turf));

  // Bottom flat platform
  sceneObjects.push_back(ShapeFactory::createCube(
      glm::vec3(33.0f, 0.7f, 24.5f), glm::vec3(6.0f, 0.3f, 3.0f), glm::vec3(0.0f), turf));

  addBorder(sceneObjects, glm::vec3(33.0f, 1.0f, 26.0f), glm::vec3(6.0f, 0.7f, 0.4f), glm::vec3(0.0f));
  addBorder(sceneObjects, glm::vec3(30.0f, 1.0f, 24.5f), glm::vec3(0.4f, 0.7f, 3.0f), glm::vec3(0.0f));
  addBorder(sceneObjects, glm::vec3(36.0f, 1.0f, 24.5f), glm::vec3(0.4f, 0.7f, 3.0f), glm::vec3(0.0f));

  addHoleCup(sceneObjects, glm::vec3(33.0f, 0.85f, 24.5f)); // FIXED: raised slightly to sit on surface

  // =====================
  // HOLE 2 - U-shape / C-shape path
  // =====================
  addHolePath(sceneObjects, glm::vec3(5.0f, 0.2f, 20.0f),  glm::vec3(3.0f, 0.3f, 12.0f), glm::vec3(0));
  addHolePath(sceneObjects, glm::vec3(11.0f, 0.2f, 24.5f), glm::vec3(15.0f, 0.3f, 3.0f), glm::vec3(0));
  addHolePath(sceneObjects, glm::vec3(17.0f, 0.2f, 20.0f), glm::vec3(3.0f, 0.3f, 12.0f), glm::vec3(0));
  addHoleCup(sceneObjects, glm::vec3(17.0f, 0.35f, 15.5f)); // FIXED: sits on surface

  // =====================
  // HOLE 3 - Straight path near water feature
  // =====================
  addHolePath(sceneObjects, glm::vec3(30.0f, 0.2f, 10.0f), glm::vec3(3.0f, 0.3f, 10.0f), glm::vec3(0));
  addHoleCup(sceneObjects, glm::vec3(30.0f, 0.35f, 6.5f)); // FIXED: cup at far end

  // =====================
  // HOLE 4 - L-shape
  // =====================
  addHolePath(sceneObjects, glm::vec3(40.0f, 0.2f, 10.0f), glm::vec3(3.0f, 0.3f, 6.0f),  glm::vec3(0));
  addHolePath(sceneObjects, glm::vec3(45.5f, 0.2f, 7.5f),  glm::vec3(9.0f, 0.3f, 3.0f),  glm::vec3(0));
  addHoleCup(sceneObjects, glm::vec3(50.0f, 0.35f, 7.5f)); // FIXED: aligned to end of path

  // =====================
  // HOLE 5 - Bottom-left, straight with slight dogleg right
  // =====================
  addHolePath(sceneObjects, glm::vec3(5.0f, 0.2f, 35.0f),  glm::vec3(3.0f, 0.3f, 8.0f),  glm::vec3(0));
  addHolePath(sceneObjects, glm::vec3(9.0f, 0.2f, 39.5f),  glm::vec3(11.0f, 0.3f, 3.0f), glm::vec3(0));
  addHoleCup(sceneObjects, glm::vec3(14.5f, 0.35f, 39.5f));

  // =====================
  // HOLE 6 - Center-left, zigzag
  // =====================
  addHolePath(sceneObjects, glm::vec3(15.0f, 0.2f, 30.0f), glm::vec3(3.0f, 0.3f, 8.0f),  glm::vec3(0));
  addHolePath(sceneObjects, glm::vec3(19.0f, 0.2f, 26.5f), glm::vec3(11.0f, 0.3f, 3.0f), glm::vec3(0));
  addHolePath(sceneObjects, glm::vec3(25.0f, 0.2f, 30.0f), glm::vec3(3.0f, 0.3f, 8.0f),  glm::vec3(0));
  addHoleCup(sceneObjects, glm::vec3(25.0f, 0.35f, 34.0f));

  // =====================
  // HOLE 7 - Top-center, with over-cover (roof obstacle)
  // =====================
  addHolePath(sceneObjects, glm::vec3(45.0f, 0.2f, 20.0f), glm::vec3(3.0f, 0.3f, 10.0f), glm::vec3(0));
  addHolePath(sceneObjects, glm::vec3(50.0f, 0.2f, 16.5f), glm::vec3(7.0f, 0.3f, 3.0f),  glm::vec3(0));
  // Over-cover: a flat cube acting as a roof over part of the path
  sceneObjects.push_back(ShapeFactory::createCube(
      glm::vec3(45.0f, 2.5f, 20.0f), glm::vec3(3.2f, 0.3f, 6.0f), glm::vec3(0), concrete));
  // Cover support pillars
  sceneObjects.push_back(ShapeFactory::createCube(
      glm::vec3(43.5f, 1.3f, 18.0f), glm::vec3(0.3f, 2.4f, 0.3f), glm::vec3(0), concrete));
  sceneObjects.push_back(ShapeFactory::createCube(
      glm::vec3(46.5f, 1.3f, 18.0f), glm::vec3(0.3f, 2.4f, 0.3f), glm::vec3(0), concrete));
  sceneObjects.push_back(ShapeFactory::createCube(
      glm::vec3(43.5f, 1.3f, 22.0f), glm::vec3(0.3f, 2.4f, 0.3f), glm::vec3(0), concrete));
  sceneObjects.push_back(ShapeFactory::createCube(
      glm::vec3(46.5f, 1.3f, 22.0f), glm::vec3(0.3f, 2.4f, 0.3f), glm::vec3(0), concrete));
  addHoleCup(sceneObjects, glm::vec3(53.5f, 0.35f, 16.5f));

  // =====================
  // HOLE 8 - Adjacent to 7, also with over-cover
  // =====================
  addHolePath(sceneObjects, glm::vec3(55.0f, 0.2f, 22.0f), glm::vec3(3.0f, 0.3f, 10.0f), glm::vec3(0));
  // Over-cover
  sceneObjects.push_back(ShapeFactory::createCube(
      glm::vec3(55.0f, 2.5f, 22.0f), glm::vec3(3.2f, 0.3f, 6.0f), glm::vec3(0), concrete));
  sceneObjects.push_back(ShapeFactory::createCube(
      glm::vec3(53.5f, 1.3f, 20.0f), glm::vec3(0.3f, 2.4f, 0.3f), glm::vec3(0), concrete));
  sceneObjects.push_back(ShapeFactory::createCube(
      glm::vec3(56.5f, 1.3f, 20.0f), glm::vec3(0.3f, 2.4f, 0.3f), glm::vec3(0), concrete));
  sceneObjects.push_back(ShapeFactory::createCube(
      glm::vec3(53.5f, 1.3f, 24.0f), glm::vec3(0.3f, 2.4f, 0.3f), glm::vec3(0), concrete));
  sceneObjects.push_back(ShapeFactory::createCube(
      glm::vec3(56.5f, 1.3f, 24.0f), glm::vec3(0.3f, 2.4f, 0.3f), glm::vec3(0), concrete));
  addHoleCup(sceneObjects, glm::vec3(55.0f, 0.35f, 17.5f));

  // =====================
  // HOLE 9 - Right side, long path with tunnel
  // =====================
  addHolePath(sceneObjects, glm::vec3(62.0f, 0.2f, 20.0f), glm::vec3(3.0f, 0.3f, 14.0f), glm::vec3(0));
  // Tunnel section: a hollow box effect using a cover + side walls
  // Tunnel roof
  sceneObjects.push_back(ShapeFactory::createCube(
      glm::vec3(62.0f, 1.8f, 20.0f), glm::vec3(3.2f, 0.3f, 5.0f), glm::vec3(0), concrete));
  // Tunnel walls
  sceneObjects.push_back(ShapeFactory::createCube(
      glm::vec3(60.4f, 1.0f, 20.0f), glm::vec3(0.3f, 1.8f, 5.0f), glm::vec3(0), concrete));
  sceneObjects.push_back(ShapeFactory::createCube(
      glm::vec3(63.6f, 1.0f, 20.0f), glm::vec3(0.3f, 1.8f, 5.0f), glm::vec3(0), concrete));
  addHolePath(sceneObjects, glm::vec3(62.0f, 0.2f, 10.0f),  glm::vec3(3.0f, 0.3f, 4.0f),  glm::vec3(0));
  addHoleCup(sceneObjects, glm::vec3(62.0f, 0.35f, 8.0f));

  // =====================
  // HOLE 10 - Bottom-center, L-shape
  // =====================
  addHolePath(sceneObjects, glm::vec3(35.0f, 0.2f, 38.0f), glm::vec3(3.0f, 0.3f, 8.0f),  glm::vec3(0));
  addHolePath(sceneObjects, glm::vec3(40.0f, 0.2f, 42.5f), glm::vec3(7.0f, 0.3f, 3.0f),  glm::vec3(0));
  addHoleCup(sceneObjects, glm::vec3(44.0f, 0.35f, 42.5f));

  // =====================
  // HOLE 11 - Bottom area, straight
  // =====================
  addHolePath(sceneObjects, glm::vec3(50.0f, 0.2f, 38.0f), glm::vec3(3.0f, 0.3f, 10.0f), glm::vec3(0));
  addHoleCup(sceneObjects, glm::vec3(50.0f, 0.35f, 33.5f));

  // =====================
  // HOLE 12 - Bottom-right, dogleg left
  // =====================
  addHolePath(sceneObjects, glm::vec3(58.0f, 0.2f, 38.0f), glm::vec3(3.0f, 0.3f, 6.0f),  glm::vec3(0));
  addHolePath(sceneObjects, glm::vec3(54.5f, 0.2f, 35.5f), glm::vec3(10.0f, 0.3f, 3.0f), glm::vec3(0));
  addHoleCup(sceneObjects, glm::vec3(50.0f, 0.35f, 35.5f));

  // =====================
  // HOLE 13 - Right side, long straight
  // =====================
  addHolePath(sceneObjects, glm::vec3(68.0f, 0.2f, 25.0f), glm::vec3(3.0f, 0.3f, 16.0f), glm::vec3(0));
  addHoleCup(sceneObjects, glm::vec3(68.0f, 0.35f, 17.5f));

  // =====================
  // HOLE 14 - Top-center, with slope (incline)
  // =====================
  addHolePath(sceneObjects, glm::vec3(22.0f, 0.2f, 5.0f),  glm::vec3(3.0f, 0.3f, 6.0f),  glm::vec3(0));
  // Inclined section
  sceneObjects.push_back(ShapeFactory::createCube(
      glm::vec3(22.0f, 0.55f, 9.2f), glm::vec3(3.0f, 0.3f, 3.5f),
      glm::vec3(8.0f, 0.0f, 0.0f), turf));
  addHolePath(sceneObjects, glm::vec3(22.0f, 0.9f, 12.5f), glm::vec3(3.0f, 0.3f, 4.0f),  glm::vec3(0));
  addHoleCup(sceneObjects, glm::vec3(22.0f, 1.05f, 12.5f));

  // =====================
  // HOLE 15 - Center, over-cover
  // =====================
  addHolePath(sceneObjects, glm::vec3(32.0f, 0.2f, 30.0f), glm::vec3(3.0f, 0.3f, 8.0f),  glm::vec3(0));
  sceneObjects.push_back(ShapeFactory::createCube(
      glm::vec3(32.0f, 2.5f, 30.0f), glm::vec3(3.2f, 0.3f, 5.0f), glm::vec3(0), concrete));
  sceneObjects.push_back(ShapeFactory::createCube(
      glm::vec3(30.4f, 1.3f, 28.5f), glm::vec3(0.3f, 2.4f, 0.3f), glm::vec3(0), concrete));
  sceneObjects.push_back(ShapeFactory::createCube(
      glm::vec3(33.6f, 1.3f, 28.5f), glm::vec3(0.3f, 2.4f, 0.3f), glm::vec3(0), concrete));
  sceneObjects.push_back(ShapeFactory::createCube(
      glm::vec3(30.4f, 1.3f, 31.5f), glm::vec3(0.3f, 2.4f, 0.3f), glm::vec3(0), concrete));
  sceneObjects.push_back(ShapeFactory::createCube(
      glm::vec3(33.6f, 1.3f, 31.5f), glm::vec3(0.3f, 2.4f, 0.3f), glm::vec3(0), concrete));
  addHoleCup(sceneObjects, glm::vec3(32.0f, 0.35f, 26.5f));

  // =====================
  // HOLE 16 - Top-left, near water feature
  // =====================
  addHolePath(sceneObjects, glm::vec3(8.0f, 0.2f, 5.0f),   glm::vec3(3.0f, 0.3f, 8.0f),  glm::vec3(0));
  addHolePath(sceneObjects, glm::vec3(12.5f, 0.2f, 9.5f),  glm::vec3(7.0f, 0.3f, 3.0f),  glm::vec3(0));
  addHoleCup(sceneObjects, glm::vec3(16.5f, 0.35f, 9.5f));

  // Water feature near holes 3 & 16 (blue flat plane)
  sceneObjects.push_back(ShapeFactory::createCube(
      glm::vec3(12.0f, 0.15f, 14.0f), glm::vec3(8.0f, 0.1f, 6.0f), glm::vec3(0), water));

  // =====================
  // HOLE 17 - Center-bottom
  // =====================
  addHolePath(sceneObjects, glm::vec3(42.0f, 0.2f, 32.0f), glm::vec3(3.0f, 0.3f, 6.0f),  glm::vec3(0));
  addHolePath(sceneObjects, glm::vec3(38.0f, 0.2f, 29.5f), glm::vec3(5.0f, 0.3f, 3.0f),  glm::vec3(0));
  addHoleCup(sceneObjects, glm::vec3(35.5f, 0.35f, 29.5f));

  // =====================
  // HOLE 18 - Bottom-left, finishing hole, slight dogleg
  // =====================
  addHolePath(sceneObjects, glm::vec3(8.0f, 0.2f, 42.0f),  glm::vec3(3.0f, 0.3f, 6.0f),  glm::vec3(0));
  addHolePath(sceneObjects, glm::vec3(13.0f, 0.2f, 38.5f), glm::vec3(7.0f, 0.3f, 3.0f),  glm::vec3(0));
  addHolePath(sceneObjects, glm::vec3(17.0f, 0.2f, 34.5f), glm::vec3(3.0f, 0.3f, 5.0f),  glm::vec3(0));
  addHoleCup(sceneObjects, glm::vec3(17.0f, 0.35f, 32.0f));

  
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

    terrain->draw(view, projection);

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

  glfwTerminate();
  return 0;
}
