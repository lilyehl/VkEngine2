#pragma once

#include "vk_context.hpp"
#include <GLFW/glfw3.h>

class VkEngine {
public:
  void init(GLFWwindow &window);

private:
  VkContext *VkContext;
};
