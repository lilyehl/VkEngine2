#include "window.hpp"
#include "GLFW/glfw3.h"

// to be figured out later
void framebufferResizeCallback(GLFWwindow *window, int, int) {
  auto engine = reinterpret_cast<Window *>(
      glfwGetWindowUserPointer(window)); // engine->framebufferResized = true;
}

void mouse_callback(GLFWwindow *window, double xposIn, double yposIn) {
  // retrieve camera object from mouse user pointer
  Camera *camera = static_cast<Camera *>(glfwGetWindowUserPointer(window));
  float xpos = static_cast<float>(xposIn);
  float ypos = static_cast<float>(yposIn);

  if (firstMouse) {
    lastX = xpos;
    lastY = ypos;
    firstMouse = false;
  }

  float xoffset = xpos - lastX;
  float yoffset =
      lastY - ypos; // reversed since y-coordinates go from bottom to top

  lastX = xpos;
  lastY = ypos;

  camera->ProcessMouseMovement(xoffset, yoffset);
}

void Window::init(Camera *camera) {
  glfwInit();

  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

  window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);
  glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
  glfwSetCursorPosCallback(window, mouse_callback);
  // glfwSetScrollCallback(window, scroll_callback);

  // store camera pointer in the window
  glfwSetWindowUserPointer(window, camera);
}

Window::~Window() {
  glfwDestroyWindow(window);
  glfwTerminate();
}

void Window::update() {
  Camera *camera = static_cast<Camera *>(glfwGetWindowUserPointer(window));
  float currentFrame = static_cast<float>(glfwGetTime());
  deltaTime = currentFrame - lastFrame + 0.25f;
  lastFrame = currentFrame;

  camera->updateProjectionMatrix(WIDTH, HEIGHT);
  camera->updateViewMatrix();

  glfwPollEvents();
  process_input();
}

bool Window::should_close() { return glfwWindowShouldClose(window); }

void Window::process_input() {
  // retrieve camera object from mouse user pointer
  Camera *camera = static_cast<Camera *>(glfwGetWindowUserPointer(window));

  if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    glfwSetWindowShouldClose(window, true);
  if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
    camera->ProcessKeyboard(FORWARD, deltaTime);
  if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    camera->ProcessKeyboard(BACKWARD, deltaTime);
  if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
    camera->ProcessKeyboard(LEFT, deltaTime);
  if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    camera->ProcessKeyboard(RIGHT, deltaTime);
}
