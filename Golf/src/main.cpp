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

void renderScene(std::vector<SceneObject> &sceneObjects, GLuint shader,
                 glm::mat4 view, glm::mat4 projection) {

  glUseProgram(shader);

  for (SceneObject &object : sceneObjects) {

    ShapeFactory::drawObject(object, shader, view, projection);
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

  camera = new Camera(glm::vec3(0.0f, 2.0f, 5.0f), -90.0f, 0.0f);

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

  GLuint objectShader = LoadShaders("object.vert", "object.frag");
  GLuint hudShader = LoadShaders("hud.vert", "hud.frag");
  droneShader = LoadShaders("drone_marker.vert", "drone_marker.frag");
  initDroneMarker();

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
  // hole 10
  // hole 11
  // hole 12
  // hole 13
  // hole 14
  // hole 15
  // hole 16
  // hole 17
  // hole 18
  while (!glfwWindowShouldClose(window)) {
    glfwGetFramebufferSize(window, &fbWidth, &fbHeight);
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

    // Main camera

    glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);

    glm::mat4 view = camera->getViewMatrix();

    glm::mat4 projection =
        camera->getProjectionMatrix((float)WINDOW_WIDTH / (float)WINDOW_HEIGHT);

    renderScene(sceneObjects, objectShader, view, projection);

    glm::mat4 skyboxView = glm::mat4(glm::mat3(view));

    skybox->draw(skyboxView, projection, isNight);
    terrain->draw();
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
      windmill->draw();
    }

    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  glfwTerminate();
  return 0;
}
