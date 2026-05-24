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

// ─── BORDER HELPERS ──────────────────────────────────────────────────────────

void addBorder(std::vector<SceneObject> &scene, glm::vec3 pos, glm::vec3 scale, glm::vec3 rot) {
    scene.push_back(ShapeFactory::createCube(pos, scale, rot, border));
}

// Full bordered path — left + right long sides + front + back caps
void addHolePath(std::vector<SceneObject> &scene, glm::vec3 pos, glm::vec3 scale, glm::vec3 rot) {
    scene.push_back(ShapeFactory::createCube(pos, scale, rot, turf));
    float hw = scale.x * 0.5f + 0.2f;
    float hd = scale.z * 0.5f + 0.2f;
    // Long sides (along Z)
    addBorder(scene, pos + glm::vec3(-hw,  0.3f, 0),   glm::vec3(0.4f, 0.6f, scale.z + 0.4f), rot);
    addBorder(scene, pos + glm::vec3( hw,  0.3f, 0),   glm::vec3(0.4f, 0.6f, scale.z + 0.4f), rot);
    // End caps (along X)
    addBorder(scene, pos + glm::vec3(0, 0.3f, -hd),   glm::vec3(scale.x + 0.8f, 0.6f, 0.4f), rot);
    addBorder(scene, pos + glm::vec3(0, 0.3f,  hd),   glm::vec3(scale.x + 0.8f, 0.6f, 0.4f), rot);
}

// Path with only LEFT+RIGHT borders — use for vertical arms that connect top+bottom
void addHolePathV(std::vector<SceneObject> &scene, glm::vec3 pos, glm::vec3 scale, glm::vec3 rot) {
    scene.push_back(ShapeFactory::createCube(pos, scale, rot, turf));
    float hw = scale.x * 0.5f + 0.2f;
    addBorder(scene, pos + glm::vec3(-hw, 0.3f, 0), glm::vec3(0.4f, 0.6f, scale.z), rot);
    addBorder(scene, pos + glm::vec3( hw, 0.3f, 0), glm::vec3(0.4f, 0.6f, scale.z), rot);
}

// Path with only FRONT+BACK borders — use for horizontal connectors
void addHolePathH(std::vector<SceneObject> &scene, glm::vec3 pos, glm::vec3 scale, glm::vec3 rot) {
    scene.push_back(ShapeFactory::createCube(pos, scale, rot, turf));
    float hd = scale.z * 0.5f + 0.2f;
    addBorder(scene, pos + glm::vec3(0, 0.3f, -hd), glm::vec3(scale.x, 0.6f, 0.4f), rot);
    addBorder(scene, pos + glm::vec3(0, 0.3f,  hd), glm::vec3(scale.x, 0.6f, 0.4f), rot);
}

// Bare surface — no borders, used inside junctions
void addHolePathBare(std::vector<SceneObject> &scene, glm::vec3 pos, glm::vec3 scale, glm::vec3 rot) {
    scene.push_back(ShapeFactory::createCube(pos, scale, rot, turf));
}

void addHoleCup(std::vector<SceneObject> &scene, glm::vec3 pos) {
    scene.push_back(ShapeFactory::createCylinder(pos, 0.22f, 0.12f, 32, glm::vec3(0), black));
}

// Over-cover: roof slab + 4 corner pillars
void addOverCover(std::vector<SceneObject> &scene, glm::vec3 center, float wx, float wz) {
    float hw = wx * 0.5f, hd = wz * 0.5f;
    // Roof slab
    scene.push_back(ShapeFactory::createCube(center + glm::vec3(0, 2.6f, 0), glm::vec3(wx + 0.3f, 0.3f, wz + 0.3f), glm::vec3(0), concrete));
    // 4 pillars
    glm::vec3 ps[4] = {
        {center.x - hw, center.y, center.z - hd},
        {center.x + hw, center.y, center.z - hd},
        {center.x - hw, center.y, center.z + hd},
        {center.x + hw, center.y, center.z + hd}
    };
    for (auto &p : ps)
        scene.push_back(ShapeFactory::createCube(p + glm::vec3(0, 1.3f, 0), glm::vec3(0.4f, 2.6f, 0.4f), glm::vec3(0), concrete));
}

// Tunnel section: roof + two side walls (open ends)
void addTunnel(std::vector<SceneObject> &scene, glm::vec3 center, float wx, float wz) {
    scene.push_back(ShapeFactory::createCube(center + glm::vec3(0, 1.9f, 0), glm::vec3(wx + 0.4f, 0.3f, wz), glm::vec3(0), concrete));
    scene.push_back(ShapeFactory::createCube(center + glm::vec3(-(wx*0.5f+0.2f), 1.0f, 0), glm::vec3(0.4f, 1.8f, wz), glm::vec3(0), concrete));
    scene.push_back(ShapeFactory::createCube(center + glm::vec3( (wx*0.5f+0.2f), 1.0f, 0), glm::vec3(0.4f, 1.8f, wz), glm::vec3(0), concrete));
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
