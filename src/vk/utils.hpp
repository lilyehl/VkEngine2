#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include "vulkan/vulkan.hpp"
#include <vulkan/vulkan_raii.hpp>

#include <cstdlib>
#include <iostream>

#include <cmath>
#include <vector>

#define LOG(msg) std::cout << msg << std::endl;

constexpr int MAX_FRAMES_IN_FLIGHT = 2;
