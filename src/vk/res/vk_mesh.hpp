#pragma once

#include "utils.hpp"
#include "vk_buffers.hpp"
#include "vk_context.hpp"
#include "vk_vertex.hpp"

struct MeshData {
  Vertices vertices;
  std::vector<uint16_t> indices;
};

struct Mesh {

  vk::raii::Buffer vertexBuffer = nullptr;
  vk::raii::DeviceMemory vertexBufferMemory = nullptr;
  vk::raii::Buffer indexBuffer = nullptr;
  vk::raii::DeviceMemory indexBufferMemory = nullptr;
  const MeshData *meshData = nullptr; // for the pipeline

  Mesh() = default;

  Mesh(VkContext &vkContext, MeshData &md) {

    // create vertex/index buffers
    vk::DeviceSize bufferSize = sizeof(md.vertices[0]) * md.vertices.size();
    Buffer::create_vertex_buffer(vkContext, vertexBuffer, vertexBufferMemory,
                                 md.vertices);
    Buffer::create_index_buffer(vkContext, indexBuffer, indexBufferMemory,
                                md.indices);
    this->meshData = &md;
  }
};
