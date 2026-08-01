#pragma once

#include "utils.hpp"
#include "vk_context.hpp"

namespace Buffer {
static std::pair<vk::raii::Buffer, vk::raii::DeviceMemory>
create_buffer(VkContext &vkContext, vk::DeviceSize size,
              vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties);

void copy_buffer(VkContext &vkContext, vk::raii::Buffer &srcBuffer,
                 vk::raii::Buffer &dstBuffer, vk::DeviceSize size);

void create_vertex_buffer(VkContext &vkContext, vk::raii::Buffer &buffer,
                          vk::raii::DeviceMemory &bufferMemory,
                          std::vector<Vertex> &vertices);

void create_index_buffer(VkContext &vkContext, vk::raii::Buffer &buffer,
                         vk::raii::DeviceMemory &bufferMemory,
                         std::vector<uint16_t> &indices);

void create_uniform_buffers(
    VkContext &vkContext, vk::DeviceSize bufferSize,
    std::vector<vk::raii::Buffer> &uniformBuffers,
    std::vector<vk::raii::DeviceMemory> &uniformBuffersMemory,
    std::vector<void *> &uniformBuffersMapped);

}; // namespace Buffer
