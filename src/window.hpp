#pragma once
#include "GLFW/glfw3.h"
#include "camera.hpp"

const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;

static bool firstMouse = true;
static float lastX = WIDTH / 2.0f;
static float lastY = HEIGHT / 2.0f;

static void framebufferResizeCallback(GLFWwindow *window, int, int);

static void mouse_callback(GLFWwindow *window, double xposIn, double yposIn);

class Window {
public:
  GLFWwindow *window;

  void init(Camera *camera);
  bool should_close();
  void update();

  Window() {}
  ~Window();

private:
  float lastFrame = 0.0f;
  float deltaTime = 0.0f;

  void process_input();
};
