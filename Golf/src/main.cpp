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
#include "Skybox.h"
#include "shader.hpp"

using namespace glm;
using namespace std;

Camera *camera;

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

  //glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

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

    glm::mat4 skyboxView = glm::mat4(glm::mat3(view));

    skybox->draw(skyboxView, projection, isNight);

    GLenum err;
  while ((err = glGetError()) != GL_NO_ERROR) {
    cout << "OpenGL Error: " << err << endl;
  }

    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  glfwTerminate();
  return 0;
}
