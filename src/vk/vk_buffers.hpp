#pragma once

#include "utils.hpp"
#include "vk_context.hpp"

using Vert = glm::vec3; // in case I want to make it a struct later
using Vertices = std::vector<glm::vec3>;

namespace Vertex {

static vk::VertexInputBindingDescription getBindingDescription() {
  // describes at which rate to load data from memory throughout vertices
  return {.binding = 0,
          .stride = sizeof(Vert),
          .inputRate = vk::VertexInputRate::eVertex}; // per-vertex data
}

static std::array<vk::VertexInputAttributeDescription, 1>
getAttributeDescriptions() {
  // describes how to extraxt a vertex attribute from a chunk of vertex data
  return {{
      // position attribute
      {.location = 0,
       .binding = 0,
       .format = vk::Format::eR32G32B32Sfloat,
       .offset = 0},
  }};
}
} // namespace Vertex

namespace Buffer {
static std::pair<vk::raii::Buffer, vk::raii::DeviceMemory>
create_buffer(VkContext &vkContext, vk::DeviceSize size,
              vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties);

void copy_buffer(VkContext &vkContext, vk::raii::Buffer &srcBuffer,
                 vk::raii::Buffer &dstBuffer, vk::DeviceSize size);

void create_vertex_buffer(VkContext &vkContext, vk::raii::Buffer &buffer,
                          vk::raii::DeviceMemory &bufferMemory,
                          Vertices &vertices);

void create_index_buffer(VkContext &vkContext, vk::raii::Buffer &buffer,
                         vk::raii::DeviceMemory &bufferMemory,
                         std::vector<uint16_t> &indices);

void create_uniform_buffers(
    VkContext &vkContext, vk::DeviceSize bufferSize,
    std::vector<vk::raii::Buffer> &uniformBuffers,
    std::vector<vk::raii::DeviceMemory> &uniformBuffersMemory,
    std::vector<void *> &uniformBuffersMapped);

}; // namespace Buffer
