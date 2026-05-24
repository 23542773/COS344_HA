#ifndef CAMERA_H
#define CAMERA_H

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

enum Camera_Movement {
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT,
    UP,
    DOWN
};

class Camera {
public:
    Camera(glm::vec3 position, float yaw, float pitch);

    glm::mat4 getViewMatrix();
    glm::mat4 getProjectionMatrix(float aspectRatio);

    void processKeyboard(Camera_Movement direction, float deltaTime);
    void processMouseMovement(float xoffset, float yoffset);
    void processMouseScroll(float yoffset);

    void setAspectRatio(float aspect);

    // orbit
    void setOrbitMode(bool enabled);
    bool isOrbiting() const;
    void setOrbitTarget(glm::vec3 target);

    glm::vec3 Position;

    float MovementSpeed;
    float MouseSensitivity;
    float Zoom; // FOV

    // ADD THESE GETTERS
    glm::vec3 getFront() const { return Front; }
    glm::vec3 getUp() const { return Up; }
    glm::vec3 getRight() const { return Right; }

private:
    void updateCameraVectors();
    void updateOrbitPosition();

    float Yaw;
    float Pitch;

    glm::vec3 Front;
    glm::vec3 Up;
    glm::vec3 Right;
    glm::vec3 WorldUp;

    float aspectRatio;

    bool orbitMode;
    glm::vec3 orbitTarget;
    float orbitDistance;
};

#endif