#pragma once

#include "utils.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// camera class from the learnopengl.com tutorial by Joey de Vries
enum Camera_Movement { FORWARD, BACKWARD, LEFT, RIGHT };

// Default camera values
const float YAW = -90.0f; // y axis rotation
const float PITCH = 0.0f; // x axis rotation
const float SPEED = 0.03f;
const float SENSITIVITY = 0.05f;
const float ZOOM = 45.0f;

// update view and projection matrice
class Camera {
public:
  glm::mat4 Mprojection = glm::mat4(1.0f);
  glm::mat4 Mview = glm::mat4(1.0f);
  // camera Attributes
  glm::vec3 Position;
  glm::vec3 Front;
  glm::vec3 Up;
  glm::vec3 Right;
  glm::vec3 WorldUp;
  // euler Angles
  float Yaw;
  float Pitch;
  // camera options
  float MovementSpeed;
  float MouseSensitivity;
  float Zoom;

  Camera(glm::vec3 position = glm::vec3(0.0f),
         glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), float yaw = YAW,
         float pitch = PITCH);
  Camera(float posX, float posY, float posZ, float upX, float upY, float upZ,
         float yaw, float pitch);

  void updateViewMatrix();
  void updateProjectionMatrix(float winWidth, float winHeight);

  void ProcessKeyboard(Camera_Movement direction, float deltaTime);

  void ProcessMouseMovement(float xoffset, float yoffset,
                            GLboolean constrainPitch = true);

  void ProcessMouseScroll(float yoffset);

private:
  // calculates the front vector from the Camera's (updated) Euler Angles
  void updateCameraVectors();
};
