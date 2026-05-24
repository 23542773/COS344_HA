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
#include "lighting/LightManager.h"
#include "shader.hpp"
#include "utils/ObjectLoader.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

using namespace glm;
using namespace std;

int fbWidth = 1280;
int fbHeight = 720;
Camera *camera;
Terrain *terrain;
Mesh *windmill;

float lastX = 640.0f;
float lastY = 360.0f;
bool firstMouse = true;

bool isNight = false;
Skybox *skybox;
GLuint hudVAO, hudVBO;
GLuint droneVAO, droneVBO;
GLuint droneShader;

GLuint sceneFBO;
GLuint sceneTexture;
GLuint sceneDepthRBO;
GLuint screenVAO, screenVBO;
GLuint screenShader;

// Global LightManager for lighting control
LightManager g_lightManager;

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
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glDepthFunc(GL_LESS);

  // glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

  return window;
}
void initFramebuffer(int width, int height) {
  glGenFramebuffers(1, &sceneFBO);
  glBindFramebuffer(GL_FRAMEBUFFER, sceneFBO);

  // Color texture
  glGenTextures(1, &sceneTexture);
  glBindTexture(GL_TEXTURE_2D, sceneTexture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB,
               GL_UNSIGNED_BYTE, NULL);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                         sceneTexture, 0);

  // Depth buffer
  glGenRenderbuffers(1, &sceneDepthRBO);
  glBindRenderbuffer(GL_RENDERBUFFER, sceneDepthRBO);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT,
                            GL_RENDERBUFFER, sceneDepthRBO);

  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    std::cout << "Framebuffer not complete!\n";

  glBindFramebuffer(GL_FRAMEBUFFER, 0);
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

void addBorder(std::vector<SceneObject> &scene, glm::vec3 pos, glm::vec3 scale,
               glm::vec3 rot) {
  scene.push_back(ShapeFactory::createCube(pos, scale, rot, border));
}

// Full bordered path — left + right long sides + front + back caps
void addHolePath(std::vector<SceneObject> &scene, glm::vec3 pos,
                 glm::vec3 scale, glm::vec3 rot) {
  scene.push_back(ShapeFactory::createCube(pos, scale, rot, turf));
  float hw = scale.x * 0.5f + 0.2f;
  float hd = scale.z * 0.5f + 0.2f;
  // Long sides (along Z)
  addBorder(scene, pos + glm::vec3(-hw, 0.3f, 0),
            glm::vec3(0.4f, 0.6f, scale.z + 0.4f), rot);
  addBorder(scene, pos + glm::vec3(hw, 0.3f, 0),
            glm::vec3(0.4f, 0.6f, scale.z + 0.4f), rot);
  // End caps (along X)
  addBorder(scene, pos + glm::vec3(0, 0.3f, -hd),
            glm::vec3(scale.x + 0.8f, 0.6f, 0.4f), rot);
  addBorder(scene, pos + glm::vec3(0, 0.3f, hd),
            glm::vec3(scale.x + 0.8f, 0.6f, 0.4f), rot);
}

// Path with only LEFT+RIGHT borders — use for vertical arms that connect
// top+bottom
void addHolePathV(std::vector<SceneObject> &scene, glm::vec3 pos,
                  glm::vec3 scale, glm::vec3 rot) {
  scene.push_back(ShapeFactory::createCube(pos, scale, rot, turf));
  float hw = scale.x * 0.5f + 0.2f;
  addBorder(scene, pos + glm::vec3(-hw, 0.3f, 0),
            glm::vec3(0.4f, 0.6f, scale.z), rot);
  addBorder(scene, pos + glm::vec3(hw, 0.3f, 0), glm::vec3(0.4f, 0.6f, scale.z),
            rot);
}

// Path with only FRONT+BACK borders — use for horizontal connectors
void addHolePathH(std::vector<SceneObject> &scene, glm::vec3 pos,
                  glm::vec3 scale, glm::vec3 rot) {
  scene.push_back(ShapeFactory::createCube(pos, scale, rot, turf));
  float hd = scale.z * 0.5f + 0.2f;
  addBorder(scene, pos + glm::vec3(0, 0.3f, -hd),
            glm::vec3(scale.x, 0.6f, 0.4f), rot);
  addBorder(scene, pos + glm::vec3(0, 0.3f, hd), glm::vec3(scale.x, 0.6f, 0.4f),
            rot);
}

// Bare surface — no borders, used inside junctions
void addHolePathBare(std::vector<SceneObject> &scene, glm::vec3 pos,
                     glm::vec3 scale, glm::vec3 rot) {
  scene.push_back(ShapeFactory::createCube(pos, scale, rot, turf));
}

void addHoleCup(std::vector<SceneObject> &scene, glm::vec3 pos) {
  scene.push_back(
      ShapeFactory::createCylinder(pos, 0.22f, 0.12f, 32, glm::vec3(0), black));
}

// Over-cover: roof slab + 4 corner pillars
void addOverCover(std::vector<SceneObject> &scene, glm::vec3 center, float wx,
                  float wz) {
  float hw = wx * 0.5f, hd = wz * 0.5f;
  // Roof slab
  scene.push_back(ShapeFactory::createCube(
      center + glm::vec3(0, 2.6f, 0), glm::vec3(wx + 0.3f, 0.3f, wz + 0.3f),
      glm::vec3(0), concrete));
  // 4 pillars
  glm::vec3 ps[4] = {{center.x - hw, center.y, center.z - hd},
                     {center.x + hw, center.y, center.z - hd},
                     {center.x - hw, center.y, center.z + hd},
                     {center.x + hw, center.y, center.z + hd}};
  for (auto &p : ps)
    scene.push_back(ShapeFactory::createCube(p + glm::vec3(0, 1.3f, 0),
                                             glm::vec3(0.4f, 2.6f, 0.4f),
                                             glm::vec3(0), concrete));
}

// Tunnel section: roof + two side walls (open ends)
void addTunnel(std::vector<SceneObject> &scene, glm::vec3 center, float wx,
               float wz) {
  scene.push_back(ShapeFactory::createCube(center + glm::vec3(0, 1.9f, 0),
                                           glm::vec3(wx + 0.4f, 0.3f, wz),
                                           glm::vec3(0), concrete));
  scene.push_back(ShapeFactory::createCube(
      center + glm::vec3(-(wx * 0.5f + 0.2f), 1.0f, 0),
      glm::vec3(0.4f, 1.8f, wz), glm::vec3(0), concrete));
  scene.push_back(ShapeFactory::createCube(
      center + glm::vec3((wx * 0.5f + 0.2f), 1.0f, 0),
      glm::vec3(0.4f, 1.8f, wz), glm::vec3(0), concrete));
}
const int WINDOW_WIDTH = 1280;
const int WINDOW_HEIGHT = 720;

const int HUD_X = fbWidth * 0.02f;
const int HUD_Y = fbHeight * 0.02f;
const int HUD_WIDTH = fbWidth * 0.2f;
const int HUD_HEIGHT = fbHeight * 0.2f;

glm::vec4 hudTint(0.3f, 0.3f, 0.3f, 0.25f);

glm::vec4 hudBorderColor(0.8f, 0.8f, 0.8f, 1.0f);

glm::vec4 droneMarkerColor(1.0f, 0.2f, 0.2f, 1.0f);

void initDroneMarker() {
  float tri[] = {0.0f, 1.0f, -0.6f, -0.6f, 0.6f, -0.6f};

  glGenVertexArrays(1, &droneVAO);
  glGenBuffers(1, &droneVBO);

  glBindVertexArray(droneVAO);

  glBindBuffer(GL_ARRAY_BUFFER, droneVBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(tri), tri, GL_STATIC_DRAW);

  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void *)0);
  glEnableVertexAttribArray(0);

  glBindVertexArray(0);
}

void drawFilledRect(float x, float y, float width, float height,
                    glm::vec4 color) {

  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();
  glOrtho(0, WINDOW_WIDTH, 0, WINDOW_HEIGHT, -1, 1);

  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glLoadIdentity();

  glDisable(GL_DEPTH_TEST);

  glColor4f(color.r, color.g, color.b, color.a);

  glBegin(GL_QUADS);

  glVertex2f(x, y);
  glVertex2f(x + width, y);
  glVertex2f(x + width, y + height);
  glVertex2f(x, y + height);

  glEnd();

  glEnable(GL_DEPTH_TEST);

  glPopMatrix();

  glMatrixMode(GL_PROJECTION);
  glPopMatrix();

  glMatrixMode(GL_MODELVIEW);
}

void drawRectOutline(float x, float y, float width, float height,
                     float thickness, glm::vec4 color) {

  drawFilledRect(x, y, width, thickness, color);

  drawFilledRect(x, y + height - thickness, width, thickness, color);

  drawFilledRect(x, y, thickness, height, color);

  drawFilledRect(x + width - thickness, y, thickness, height, color);
}

void drawDroneMarker(float centerX, float centerY, float size) {
  glUseProgram(droneShader);

  glBindVertexArray(droneVAO);

  glDisable(GL_DEPTH_TEST);

  glUniform2f(glGetUniformLocation(droneShader, "uOffset"),
              centerX - size * 0.5f, centerY - size * 0.5f);

  glUniform2f(glGetUniformLocation(droneShader, "uScale"), size, size);

  glUniform2f(glGetUniformLocation(droneShader, "uResolution"),
              (float)WINDOW_WIDTH, (float)WINDOW_HEIGHT);

  glUniform4f(glGetUniformLocation(droneShader, "uColor"), droneMarkerColor.r,
              droneMarkerColor.g, droneMarkerColor.b, droneMarkerColor.a);

  glDrawArrays(GL_TRIANGLES, 0, 3);

  glBindVertexArray(0);

  glEnable(GL_DEPTH_TEST);
}

bool collide(const glm::vec3 &position, float radius, const SceneObject &obj) {
  if (obj.collider.type == ColliderType::AABB) {
    glm::vec3 minB = obj.collider.center - obj.collider.halfSize;
    glm::vec3 maxB = obj.collider.center + obj.collider.halfSize;

    glm::vec3 closest(glm::max(minB.x, glm::min(position.x, maxB.x)),
                      glm::max(minB.y, glm::min(position.y, maxB.y)),
                      glm::max(minB.z, glm::min(position.z, maxB.z)));

    return glm::length(position - closest) < radius;
  }

  if (obj.collider.type == ColliderType::SPHERE) {
    return glm::length(position - obj.collider.center) <
           (radius + obj.collider.radius);
  }

  return false;
}
void initScreenQuad() {
  float quad[] = {-1.0f, 1.0f, -1.0f, -1.0f, 1.0f, -1.0f,

                  -1.0f, 1.0f, 1.0f,  -1.0f, 1.0f, 1.0f};

  glGenVertexArrays(1, &screenVAO);
  glGenBuffers(1, &screenVBO);

  glBindVertexArray(screenVAO);
  glBindBuffer(GL_ARRAY_BUFFER, screenVBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(quad), quad, GL_STATIC_DRAW);

  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void *)0);
  glEnableVertexAttribArray(0);
}

void renderScene(std::vector<SceneObject> &sceneObjects, Shader &shader,
                 glm::mat4 view, glm::mat4 projection) {

  shader.use();

  // Bind lighting uniforms
  g_lightManager.bindAllLights(shader);
  shader.setVec3("viewPos", camera->Position);

  GLuint shaderID = shader.getProgramID();

  // Set matrices for ShapeFactory
  GLint viewLoc = glGetUniformLocation(shaderID, "view");
  GLint projLoc = glGetUniformLocation(shaderID, "projection");
  glUniformMatrix4fv(viewLoc, 1, GL_FALSE, &view[0][0]);
  glUniformMatrix4fv(projLoc, 1, GL_FALSE, &projection[0][0]);

  for (SceneObject &object : sceneObjects) {
    ShapeFactory::drawObject(object, shaderID, view, projection);
  }
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
  float angle = camera->getYaw();

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

  // Use Shader class instead of GLuint for lighting support
  Shader objectShader("object.vert", "object.frag");
  GLuint hudShader = LoadShaders("hud.vert", "hud.frag");
  droneShader = LoadShaders("drone_marker.vert", "drone_marker.frag");
  screenShader = LoadShaders("nightvision.vert", "nightvision.frag");
  initDroneMarker();
  initScreenQuad();

  g_lightManager.getDroneLight().diffuse =
      glm::vec3(3.0f, 3.0f, 3.0f); // 3x brighter
  g_lightManager.getDroneLight().ambient =
      glm::vec3(0.4f, 0.4f, 0.4f); // Higher ambient
  g_lightManager.getDroneLight().specular =
      glm::vec3(2.0f, 2.0f, 2.0f); // Brighter specular

  initFramebuffer(fbWidth, fbHeight);
  float hudVertices[] = {
      // positions    // colors
      -1.0f, 1.0f, 0.3f,  0.3f, 0.3f,  0.25f, -1.0f, -1.0f, 0.3f,
      0.3f,  0.3f, 0.25f, 1.0f, -1.0f, 0.3f,  0.3f,  0.3f,  0.25f,

      -1.0f, 1.0f, 0.3f,  0.3f, 0.3f,  0.25f, 1.0f,  -1.0f, 0.3f,
      0.3f,  0.3f, 0.25f, 1.0f, 1.0f,  0.3f,  0.3f,  0.3f,  0.25f,
  };

  glGenVertexArrays(1, &hudVAO);
  glGenBuffers(1, &hudVBO);

  glBindVertexArray(hudVAO);

  glBindBuffer(GL_ARRAY_BUFFER, hudVBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(hudVertices), hudVertices,
               GL_STATIC_DRAW);

  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)0);
  glEnableVertexAttribArray(0);

  glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 6 * sizeof(float),
                        (void *)(2 * sizeof(float)));
  glEnableVertexAttribArray(1);

  glBindVertexArray(0);

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

  //Hole 1
  float h1X = 10.0f; float h1Z = 10.0f;
  // Lower path
  addHolePath(sceneObjects, {h1X, 0.2f, h1Z}, {3.0f, 0.3f, 4.0f}, {0,0,0});
  addHoleCup(sceneObjects, {h1X, 0.35f, h1Z}); // <--- Cup now on small square
  
  // Ramp
  sceneObjects.push_back(ShapeFactory::createCube({h1X, 0.5f, h1Z + 4.0f}, {3.0f, 0.3f, 4.0f}, {15.0f, 0, 0}, turf));
  // Elevated Green
  sceneObjects.push_back(ShapeFactory::createCube({h1X, 0.8f, h1Z + 8.0f}, {6.0f, 0.3f, 6.0f}, {0,0,0}, turf));
  
  // Walls around elevated green
  addBorder(sceneObjects, {h1X - 3.2f, 1.1f, h1Z + 8.0f}, {0.4f, 0.6f, 6.0f}, {0,0,0}); 
  addBorder(sceneObjects, {h1X + 3.2f, 1.1f, h1Z + 8.0f}, {0.4f, 0.6f, 6.0f}, {0,0,0}); 
  addBorder(sceneObjects, {h1X, 1.1f, h1Z + 11.2f}, {6.8f, 0.6f, 0.4f}, {0,0,0});

  //Hole 2
  float h2X = 30.0f; float h2Z = 10.0f;
  // Paths
  addHolePathBare(sceneObjects, {h2X, 0.2f, h2Z}, {3.0f, 0.3f, 6.0f}, {0,0,0}); // Vertical
  addHolePathBare(sceneObjects, {h2X + 3.0f, 0.2f, h2Z + 1.5f}, {6.0f, 0.3f, 3.0f}, {0,0,0}); // Horizontal

  // 1. Left Wall (Vertical section only)
  addBorder(sceneObjects, {h2X - 1.7f, 0.5f, h2Z}, {0.4f, 0.6f, 6.0f}, {0,0,0});
  
  // 2. Bottom Wall (Horizontal section only)
  addBorder(sceneObjects, {h2X + 3.0f, 0.5f, h2Z - 0.2f}, {6.4f, 0.6f, 0.4f}, {0,0,0});

  // 3. Right Wall (Horizontal section end)
  addBorder(sceneObjects, {h2X + 6.2f, 0.5f, h2Z + 1.5f}, {0.4f, 0.6f, 3.0f}, {0,0,0});

  // 4. Top Wall (Horizontal section top)
  addBorder(sceneObjects, {h2X + 3.0f, 0.5f, h2Z + 3.2f}, {6.4f, 0.6f, 0.4f}, {0,0,0});
  
  addHoleCup(sceneObjects, {h2X + 5.5f, 0.35f, h2Z + 1.5f});

  //Hole 3
  float h3X = 30.0f; float h3Z = 25.0f;

  // 1. Path Segments (Bare)
  addHolePathBare(sceneObjects, {h3X, 0.2f, h3Z}, {3.0f, 0.3f, 6.0f}, {0, 20, 0});
  addHolePathBare(sceneObjects, {h3X + 2.0f, 0.2f, h3Z + 6.0f}, {3.0f, 0.3f, 6.0f}, {0, -20, 0});
  addHolePathBare(sceneObjects, {h3X + 1.0f, 0.2f, h3Z + 3.0f}, {3.0f, 0.3f, 3.5f}, {0, 0, 0});

  // 2. Borders (Blue lines: trimmed to 4.5f to prevent overhang)
  // Top-left wall
  addBorder(sceneObjects, {h3X - 1.5f, 0.5f, h3Z}, {0.4f, 0.6f, 4.5f}, {0, 20, 0}); 
  
  // Right-side walls (Split to stop at the turn)
  addBorder(sceneObjects, {h3X + 1.5f, 0.5f, h3Z}, {0.4f, 0.6f, 4.5f}, {0, 20, 0}); 
  addBorder(sceneObjects, {h3X + 3.7f, 0.5f, h3Z + 6.0f}, {0.4f, 0.6f, 4.5f}, {0, -20, 0});
  
  // Bottom-right wall
  addBorder(sceneObjects, {h3X + 0.5f, 0.5f, h3Z + 6.0f}, {0.4f, 0.6f, 4.5f}, {0, -20, 0});

  addHoleCup(sceneObjects, {h3X + 2.0f, 0.35f, h3Z + 7.5f});

  //hole 4
  float h4X = 10.0f; float h4Z = 35.0f;

  // 1. ADD BACK THE TURF (Missing in your current view)
  addHolePathBare(sceneObjects, {h4X, 0.2f, h4Z + 2.0f}, {3.0f, 0.3f, 4.0f}, {0,0,0}); // Main
  addHolePathBare(sceneObjects, {h4X - 2.5f, 0.2f, h4Z + 6.0f}, {3.0f, 0.3f, 5.0f}, {0, -15, 0}); // Left branch
  addHolePathBare(sceneObjects, {h4X + 2.5f, 0.2f, h4Z + 6.0f}, {3.0f, 0.3f, 5.0f}, {0, 15, 0}); // Right branch
  addHolePathBare(sceneObjects, {h4X, 0.2f, h4Z + 9.5f}, {8.0f, 0.3f, 4.0f}, {0,0,0}); // Rejoin
  addHoleCup(sceneObjects, {h4X, 0.35f, h4Z + 9.5f});

  // 2. THE BORDERS (The ones you already have)
  addBorder(sceneObjects, {h4X - 1.7f, 0.5f, h4Z + 2.0f}, {0.4f, 0.6f, 4.0f}, {0,0,0});
  addBorder(sceneObjects, {h4X + 1.7f, 0.5f, h4Z + 2.0f}, {0.4f, 0.6f, 4.0f}, {0,0,0});
  addBorder(sceneObjects, {h4X - 4.0f, 0.5f, h4Z + 6.0f}, {0.4f, 0.6f, 4.5f}, {0, -15, 0});
  addBorder(sceneObjects, {h4X + 4.0f, 0.5f, h4Z + 6.0f}, {0.4f, 0.6f, 4.5f}, {0, 15, 0});

  // hole 5

  float h5X = 45.0f; float h5Z = 10.0f; 

  // 1. Single Straight Path
  addHolePathBare(sceneObjects, {h5X, 0.2f, h5Z}, {4.0f, 0.3f, 12.0f}, {0, 0, 0});
  addHoleCup(sceneObjects, {h5X, 0.35f, h5Z + 5.0f});

  // 2. Clean Straight Borders
  // Left border
  addBorder(sceneObjects, {h5X - 2.2f, 0.5f, h5Z}, {0.4f, 0.6f, 12.0f}, {0, 0, 0});
  // Right border
  addBorder(sceneObjects, {h5X + 2.2f, 0.5f, h5Z}, {0.4f, 0.6f, 12.0f}, {0, 0, 0});

  //hole 6
  float h6X = 45.0f; float h6Z = 25.0f;

  // 1. Path (Wide base tapering to narrow)
  // Wide section
  addHolePathBare(sceneObjects, {h6X, 0.2f, h6Z}, {6.0f, 0.3f, 5.0f}, {0, 0, 0});
  // Narrow section
  addHolePathBare(sceneObjects, {h6X, 0.2f, h6Z + 6.5f}, {2.0f, 0.3f, 8.0f}, {0, 0, 0});
  
  addHoleCup(sceneObjects, {h6X, 0.35f, h6Z + 10.0f});

  // 2. Borders (Angled to create the funnel effect)
  // Left side
  addBorder(sceneObjects, {h6X - 3.2f, 0.5f, h6Z}, {0.4f, 0.6f, 5.0f}, {0, 0, 0}); // Wide wall
  addBorder(sceneObjects, {h6X - 1.2f, 0.5f, h6Z + 6.5f}, {0.4f, 0.6f, 8.0f}, {0, 0, 0}); // Narrow wall
  
  // Right side
  addBorder(sceneObjects, {h6X + 3.2f, 0.5f, h6Z}, {0.4f, 0.6f, 5.0f}, {0, 0, 0}); // Wide wall
  addBorder(sceneObjects, {h6X + 1.2f, 0.5f, h6Z + 6.5f}, {0.4f, 0.6f, 8.0f}, {0, 0, 0}); // Narrow wall

  float rollOffset = 0;
  static bool nPressedLast = false;
  glm::vec3 lastCamPos;
  static bool gPressedLast = false;
  bool grayscale = false;

  // F-key press tracking
  static bool f1Pressed = false;
  static bool f2Pressed = false;
  static bool f3Pressed = false;
  static bool f4Pressed = false;

  // Debug frame counter
  static int debugFrameCount = 0;

  while (!glfwWindowShouldClose(window)) {
    lastCamPos = camera->Position;
    glfwGetFramebufferSize(window, &fbWidth, &fbHeight);
    float currentFrame = glfwGetTime();
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;
    rollOffset = 0;
    bool nPressed = glfwGetKey(window, GLFW_KEY_N) == GLFW_PRESS;

    if (nPressed && !nPressedLast) {
      isNight = !isNight;
    }

    nPressedLast = nPressed;

    bool gPressed = glfwGetKey(window, GLFW_KEY_G) == GLFW_PRESS;

    if (gPressed && !gPressedLast) {
      grayscale = !grayscale;
    }

    gPressedLast = gPressed;

    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
      glfwSetWindowShouldClose(window, true);

    // ─────────────────────────────────────────────────────────────
    // LIGHTING CONTROLS (F1-F4)
    // ─────────────────────────────────────────────────────────────

    // F1: Toggle sun
    if (glfwGetKey(window, GLFW_KEY_F1) == GLFW_PRESS) {
      if (!f1Pressed) {
        g_lightManager.toggleSun();
        g_lightManager.printLightState();
        f1Pressed = true;
      }
    } else {
      f1Pressed = false;
    }

    // F2: Night mode
    if (glfwGetKey(window, GLFW_KEY_F2) == GLFW_PRESS) {
      if (!f2Pressed) {
        g_lightManager.setNightMode();
        g_lightManager.printLightState();
        f2Pressed = true;
      }
    } else {
      f2Pressed = false;
    }

    // F3: Day mode
    if (glfwGetKey(window, GLFW_KEY_F3) == GLFW_PRESS) {
      if (!f3Pressed) {
        g_lightManager.setDayMode();
        g_lightManager.printLightState();
        f3Pressed = true;
      }
    } else {
      f3Pressed = false;
    }

    // F4: Toggle drone light
    if (glfwGetKey(window, GLFW_KEY_F4) == GLFW_PRESS) {
      if (!f4Pressed) {
        g_lightManager.toggleDroneLight();
        g_lightManager.printLightState();
        f4Pressed = true;
      }
    } else {
      f4Pressed = false;
    }

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

    if (glfwGetKey(window, GLFW_KEY_EQUAL) == GLFW_PRESS)
      camera->MovementSpeed += 0.1f;

    if (glfwGetKey(window, GLFW_KEY_MINUS) == GLFW_PRESS)
      camera->MovementSpeed -= 0.1f;

    if (glfwGetKey(window, GLFW_KEY_I) == GLFW_PRESS)
      camera->Zoom += 0.5f;

    if (glfwGetKey(window, GLFW_KEY_O) == GLFW_PRESS)
      camera->Zoom -= 0.5f;

    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
      rollOffset -= 0.5;

    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
      rollOffset += 0.5;

    camera->processRoll(rollOffset);

    // ─────────────────────────────────────────────────────────────
    // UPDATE DRONE LIGHT POSITION AND DIRECTION
    // ─────────────────────────────────────────────────────────────
    // Get camera front direction (where the drone/camera is looking)
    glm::vec3 cameraFront = camera->getFront();
    glm::vec3 cameraPos = camera->Position;

    // Update drone light position (attached to camera)
    g_lightManager.getDroneLight().position = cameraPos;

    // Update drone light direction (pointing where camera looks)
    g_lightManager.getDroneLight().direction = cameraFront;

    // Debug print every 300 frames
    if (debugFrameCount++ % 300 == 0) {
      std::cout << "Drone Light Pos: (" << cameraPos.x << ", " << cameraPos.y
                << ", " << cameraPos.z << ")" << std::endl;
      std::cout << "Drone Light Dir: (" << cameraFront.x << ", "
                << cameraFront.y << ", " << cameraFront.z << ")" << std::endl;
      std::cout << "Drone Light Enabled: "
                << g_lightManager.getDroneLight().enabled << std::endl;
    }

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Main camera
    glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);

    glm::mat4 view = camera->getViewMatrix();

    glm::mat4 projection =
        camera->getProjectionMatrix((float)WINDOW_WIDTH / (float)WINDOW_HEIGHT);
    glBindFramebuffer(GL_FRAMEBUFFER, sceneFBO);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Bind lights before rendering scene
    objectShader.use();
    g_lightManager.bindAllLights(objectShader);
    objectShader.setVec3("viewPos", camera->Position);

    renderScene(sceneObjects, objectShader, view, projection);
    glm::vec3 dronePos = camera->Position;
    float droneRadius = 0.6f;

    for (const SceneObject &obj : sceneObjects) {
      if (collide(dronePos, droneRadius, obj)) {
        camera->Position = lastCamPos;
      }
    }
    glm::mat4 skyboxView = glm::mat4(glm::mat3(view));

    skybox->draw(skyboxView, projection, isNight);

    // Bind lights for terrain
    objectShader.use();
    g_lightManager.bindAllLights(objectShader);
    objectShader.setVec3("viewPos", camera->Position);

    terrain->draw(view, projection);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClear(GL_COLOR_BUFFER_BIT);
    glUseProgram(screenShader);

    glUniform1i(glGetUniformLocation(screenShader, "nightMode"), isNight);
    glUniform1i(glGetUniformLocation(screenShader, "grayscale"), grayscale);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, sceneTexture);
    glUniform1i(glGetUniformLocation(screenShader, "screenTexture"), 0);

    glBindVertexArray(screenVAO);
    glDisable(GL_DEPTH_TEST);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glEnable(GL_DEPTH_TEST);

    // HUD minimap
    glViewport(HUD_X, HUD_Y, HUD_WIDTH, HUD_HEIGHT);

    glEnable(GL_SCISSOR_TEST);

    glScissor(HUD_X, HUD_Y, HUD_WIDTH, HUD_HEIGHT);

    glClear(GL_DEPTH_BUFFER_BIT);

    glm::mat4 hudView = camera->getTopDownViewMatrix(50.0f);

    glm::mat4 hudProjection = camera->getOrthographicProjection(60.0f, 40.0f);

    renderScene(sceneObjects, objectShader, hudView, hudProjection);

    glDisable(GL_SCISSOR_TEST);

    // HUD tint
    glViewport(HUD_X, HUD_Y, HUD_WIDTH, HUD_HEIGHT);

    glUseProgram(hudShader);

    glDisable(GL_DEPTH_TEST);

    glBindVertexArray(hudVAO);

    glDrawArrays(GL_TRIANGLES, 0, 6);

    glBindVertexArray(0);

    glEnable(GL_DEPTH_TEST);

    // Restore viewport
    glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);

    // Drone marker
    drawDroneMarker(HUD_X + HUD_WIDTH * 0.5f, HUD_Y + HUD_HEIGHT * 0.5f, 10.0f);
    GLenum err;
    while ((err = glGetError()) != GL_NO_ERROR) {
      cout << "OpenGL Error: " << err << endl;
    }

    if (windmill != nullptr) {
      glm::mat4 model = glm::mat4(1.0f);
      // Bind lights for windmill
      objectShader.use();
      g_lightManager.bindAllLights(objectShader);
      objectShader.setVec3("viewPos", camera->Position);
      windmill->draw(view, projection, model);
    }

    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  glfwTerminate();
  return 0;
}
