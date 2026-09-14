#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include "vulkan/vulkan.hpp"
#include <vulkan/vulkan_raii.hpp>

#include <cstdlib>
#include <iostream>

#define LOG(msg) std::cout << msg << std::endl;

constexpr int MAX_FRAMES_IN_FLIGHT = 2;

// temp ?
struct Vertex {
  glm::vec3 pos;
  glm::vec3 color;

  static vk::VertexInputBindingDescription getBindingDescription() {
    // describes at which rate to load data from memory throughout vertices
    return {.binding = 0,
            .stride = sizeof(Vertex),
            .inputRate = vk::VertexInputRate::eVertex}; // per-vertex data
  }

  static std::array<vk::VertexInputAttributeDescription, 2>
  getAttributeDescriptions() {
    // describes how to extraxt a vertex attribute from a chunk of vertex data
    return {{// position attribute
             {.location = 0,
              .binding = 0,
              .format = vk::Format::eR32G32B32Sfloat,
              .offset = offsetof(Vertex, pos)},
             // color attribute
             {.location = 1,
              .binding = 0,
              .format = vk::Format::eR32G32B32Sfloat,
              .offset = offsetof(Vertex, color)}}};
  }
};
