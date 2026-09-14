#pragma once

#include "utils.hpp"
#include "vk_buffers.hpp"
#include "vk_context.hpp"

struct Voxel {
  // Currently, 1 buffer per cube, VERY inefficient

  // TODO : implement a mesh manager
  // The voxel struct will only contain info about vertices and the mesh manager
  // allocates it into chunks (the buffers)

  // currently cube vertices
  std::vector<Vertex> vertices = {{{-1.f, -1.f, 0.5f}, {0.0f, 1.0f, 0.0f}},
                                  {{1.f, -1.f, 0.5f}, {1.0f, 0.0f, 0.0f}},
                                  {{-1.f, 1.f, 0.5f}, {0.0f, 0.0f, 1.0f}},
                                  {{1.f, 1.f, 0.5f}, {1.0f, 1.0f, 1.0f}},
                                  {{-1.f, -1.f, -0.5f}, {1.0f, 0.0f, 0.0f}},
                                  {{1.f, -1.f, -0.5f}, {0.0f, 1.0f, 0.0f}},
                                  {{-1.f, 1.f, -0.5f}, {0.0f, 0.0f, 1.0f}},
                                  {{1.f, 1.f, -0.5f}, {1.0f, 1.0f, 1.0f}}};
  std::vector<uint16_t> indices = {
      // Top
      2, 7, 6, 2, 3, 7,
      // Bottom
      0, 4, 5, 0, 5, 1,
      // Left
      0, 2, 6, 0, 6, 4,
      // Right
      1, 7, 3, 1, 5, 7,
      // Front
      0, 3, 2, 0, 1, 3,
      // Back
      4, 6, 7, 4, 7, 5};

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
