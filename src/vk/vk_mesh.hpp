#pragma once

#include "utils.hpp"
#include "vk_buffers.hpp"
#include "vk_context.hpp"

struct Voxel {
  // Currently, 1 buffer per cube, VERY inefficient

  // TODO : implement a mesh manager
  // The voxel struct will only contain info about vertices and the mesh manager
  // allocates it into chunks (the buffers)

  // currently square vertices
  std::vector<Vertex> vertices = {{{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
                                  {{0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
                                  {{0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
                                  {{-0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}}};
  std::vector<uint16_t> indices = {0, 1, 2, 2, 3, 0};

  vk::raii::Buffer vertexBuffer = nullptr;
  vk::raii::DeviceMemory vertexBufferMemory = nullptr;
  vk::raii::Buffer indexBuffer = nullptr;
  vk::raii::DeviceMemory indexBufferMemory = nullptr;

  Voxel() = default;

  Voxel(VkContext &vkContext) {

    // create vertex buffer (to be moved)
    vk::DeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();
    Buffer::create_vertex_buffer(vkContext, vertexBuffer, vertexBufferMemory,
                                 vertices);
    Buffer::create_index_buffer(vkContext, indexBuffer, indexBufferMemory,
                                indices);
  }
};
