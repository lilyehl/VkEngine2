#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vulkan/vulkan_raii.hpp>

#include <cstdlib>
#include <iostream>

#define LOG(msg) std::cout << msg << std::endl;
