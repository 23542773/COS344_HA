#include "Camera.h"
#include <algorithm>
#include <cmath>

Camera::Camera(glm::vec3 position, float yaw, float pitch)
    : Front(0.0f, 0.0f, -1.0f),
      MovementSpeed(5.0f),
      MouseSensitivity(0.1f),
      Zoom(60.0f),
      aspectRatio(16.0f / 9.0f),
      orbitMode(false),
      orbitTarget(0.0f, 0.0f, 0.0f),
      orbitDistance(10.0f)
{
    Position = position;
    Yaw = yaw;
    Pitch = pitch;
    WorldUp = glm::vec3(0.0f, 1.0f, 0.0f);
    updateCameraVectors();
}

glm::mat4 Camera::getViewMatrix() {
    if (orbitMode)
        updateOrbitPosition();

    return glm::lookAt(Position, Position + Front, Up);
}

glm::mat4 Camera::getProjectionMatrix(float aspectRatio) {
    return glm::perspective(glm::radians(Zoom), aspectRatio, 0.1f, 1000.0f);
}

void Camera::setAspectRatio(float aspect) {
    aspectRatio = aspect;
}

void Camera::processKeyboard(Camera_Movement direction, float deltaTime) {
    float velocity = MovementSpeed * deltaTime;

    if (orbitMode) {
        if (direction == LEFT) orbitDistance += velocity;
        if (direction == RIGHT) orbitDistance -= velocity;
        return;
    }

    if (direction == FORWARD)
        Position += Front * velocity;
    if (direction == BACKWARD)
        Position -= Front * velocity;
    if (direction == LEFT)
        Position -= Right * velocity;
    if (direction == RIGHT)
        Position += Right * velocity;
    if (direction == UP)
        Position += WorldUp * velocity;
    if (direction == DOWN)
        Position -= WorldUp * velocity;
}

void Camera::processMouseMovement(float xoffset, float yoffset) {
    xoffset *= MouseSensitivity;
    yoffset *= MouseSensitivity;

    Yaw += xoffset;
    Pitch += yoffset;

    if (Pitch > 89.0f) Pitch = 89.0f;
    if (Pitch < -89.0f) Pitch = -89.0f;

    updateCameraVectors();
}

void Camera::processMouseScroll(float yoffset) {
    Zoom -= yoffset;
    if (Zoom < 20.0f) Zoom = 20.0f;
    if (Zoom > 90.0f) Zoom = 90.0f;
}

void Camera::setOrbitMode(bool enabled) {
    orbitMode = enabled;
}

bool Camera::isOrbiting() const {
    return orbitMode;
}

void Camera::updateOrbitPosition() {
    float yawRad = glm::radians(Yaw);
    float pitchRad = glm::radians(Pitch);

    Position.x = orbitTarget.x + orbitDistance * cos(pitchRad) * cos(yawRad);
    Position.y = orbitTarget.y + orbitDistance * sin(pitchRad);
    Position.z = orbitTarget.z + orbitDistance * cos(pitchRad) * sin(yawRad);

    Front = glm::normalize(orbitTarget - Position);
}

void Camera::updateCameraVectors() {
    glm::vec3 front;
    front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
    front.y = sin(glm::radians(Pitch));
    front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));

    Front = glm::normalize(front);
    Right = glm::normalize(glm::cross(Front, WorldUp));
    Up = glm::normalize(glm::cross(Right, Front));
}
