#ifndef CAMERA_H
#define CAMERA_H

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

enum Camera_Movement { FORWARD, BACKWARD, LEFT, RIGHT, UP, DOWN };

class Camera {
public:
  Camera(glm::vec3 position, float yaw, float pitch);

  glm::mat4 getViewMatrix();
  glm::mat4 getProjectionMatrix(float aspectRatio);
  glm::mat4 getTopDownViewMatrix(float height);
  glm::mat4 getOrthographicProjection(float width, float height);

  void processKeyboard(Camera_Movement direction, float deltaTime);
  void processMouseMovement(float xoffset, float yoffset);
  void processMouseScroll(float yoffset);
  void processRoll(float offset);

  float getYaw();
  glm::vec3 getFront();
  glm::vec3 getVelocity();

  void setAspectRatio(float aspect);

  void update(float deltaTime);

  // orbit
  void setOrbitMode(bool enabled);
  bool isOrbiting() const;

  glm::vec3 Position;

  float MovementSpeed;
  float MouseSensitivity;
  float Zoom; // FOV

private:
  void updateCameraVectors();
  void updateOrbitPosition();

  float Yaw;
  float Pitch;
  float Roll;

  glm::vec3 Front;
  glm::vec3 Up;
  glm::vec3 Right;
  glm::vec3 WorldUp;

  glm::vec3 Velocity;
  glm::vec3 Acceleration;

  float MaxSpeed;
  float AccelerationRate;
  float DecelerationRate;

  float aspectRatio;

  bool orbitMode;
  glm::vec3 orbitTarget;
  float orbitDistance;
};

#endif
