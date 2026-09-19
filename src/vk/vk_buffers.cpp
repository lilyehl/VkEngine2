#include "vk_buffers.hpp"

std::pair<vk::raii::Buffer, vk::raii::DeviceMemory>
Buffer::create_buffer(VkContext &vkContext, vk::DeviceSize size,
                      vk::BufferUsageFlags usage,
                      vk::MemoryPropertyFlags properties) {

  vk::BufferCreateInfo bufferInfo{
      .size = size, .usage = usage, .sharingMode = vk::SharingMode::eExclusive};
  vk::raii::Buffer buffer = vk::raii::Buffer(vkContext.device, bufferInfo);

  vk::MemoryRequirements memRequirements = buffer.getMemoryRequirements();
  vk::MemoryAllocateInfo allocInfo{
      .allocationSize = memRequirements.size,
      .memoryTypeIndex = vkContext.find_memory_type(
          memRequirements.memoryTypeBits, properties)};

  vk::raii::DeviceMemory bufferMemory =
      vk::raii::DeviceMemory(vkContext.device, allocInfo);

  buffer.bindMemory(*bufferMemory, 0);
  return {std::move(buffer), std::move(bufferMemory)};
}

void Buffer::copy_buffer(VkContext &vkContext, vk::raii::Buffer &srcBuffer,
                         vk::raii::Buffer &dstBuffer, vk::DeviceSize size) {

  // allocating a temporary command buffer
  // TODO : allocate a small, separate command pool for temp buffers (w/
  // eTransient flag for memory optimisations )
  vk::CommandBufferAllocateInfo allocInfo{.commandPool = vkContext.commandPool,
                                          .level =
                                              vk::CommandBufferLevel::ePrimary,
                                          .commandBufferCount = 1};
  vk::raii::CommandBuffer commandCopyBuffer =
      std::move(vkContext.device.allocateCommandBuffers(allocInfo).front());
  commandCopyBuffer.begin(
      {.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit});

  // copy da buffa
  commandCopyBuffer.copyBuffer(*srcBuffer, *dstBuffer,
                               vk::BufferCopy(0, 0, size));
  commandCopyBuffer.end();

  // execute da buffa
  vkContext.queue.submit(vk::SubmitInfo{.commandBufferCount = 1,
                                        .pCommandBuffers = &*commandCopyBuffer},
                         nullptr);
  vkContext.queue.waitIdle();
}

void Buffer::create_vertex_buffer(VkContext &vkContext,
                                  vk::raii::Buffer &buffer,
                                  vk::raii::DeviceMemory &bufferMemory,
                                  Vertices &vertices) {

  // create staging buffer
  vk::DeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();

  auto [stagingBuffer, stagingBufferMemory] = create_buffer(
      vkContext, bufferSize, vk::BufferUsageFlagBits::eTransferSrc,
      vk::MemoryPropertyFlagBits::eHostVisible |
          vk::MemoryPropertyFlagBits::eHostCoherent);

  // copy vertex data into staging buffer
  void *dataStaging = stagingBufferMemory.mapMemory(0, bufferSize);
  memcpy(dataStaging, vertices.data(), bufferSize);
  stagingBufferMemory.unmapMemory();

  std::tie(buffer, bufferMemory) =
      // create vertex buffer from a memory type that is device local
      create_buffer(vkContext, bufferSize,
                    vk::BufferUsageFlagBits::eVertexBuffer |
                        vk::BufferUsageFlagBits::eTransferDst,
                    vk::MemoryPropertyFlagBits::eDeviceLocal);

  // bc of mem type, we can't use mapMemory, so we copy it from staging ->
  // vertex buffer
  copy_buffer(vkContext, stagingBuffer, buffer, bufferSize);
}

void Buffer::create_index_buffer(VkContext &vkContext, vk::raii::Buffer &buffer,
                                 vk::raii::DeviceMemory &bufferMemory,
                                 std::vector<uint16_t> &indices) {
  vk::DeviceSize bufferSize = sizeof(indices[0]) * indices.size();

  auto [stagingBuffer, stagingBufferMemory] = create_buffer(
      vkContext, bufferSize, vk::BufferUsageFlagBits::eTransferSrc,
      vk::MemoryPropertyFlagBits::eHostVisible |
          vk::MemoryPropertyFlagBits::eHostCoherent);

  void *data = stagingBufferMemory.mapMemory(0, bufferSize);
  memcpy(data, indices.data(), (size_t)bufferSize);
  stagingBufferMemory.unmapMemory();

  std::tie(buffer, bufferMemory) =
      create_buffer(vkContext, bufferSize,
                    vk::BufferUsageFlagBits::eIndexBuffer |
                        vk::BufferUsageFlagBits::eTransferDst,
                    vk::MemoryPropertyFlagBits::eDeviceLocal);

  copy_buffer(vkContext, stagingBuffer, buffer, bufferSize);
}

void Buffer::create_uniform_buffers(
    VkContext &vkContext, vk::DeviceSize bufferSize,
    std::vector<vk::raii::Buffer> &uniformBuffers,
    std::vector<vk::raii::DeviceMemory> &uniformBuffersMemory,
    std::vector<void *> &uniformBuffersMapped) {
  for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
    auto [buffer, bufferMem] = create_buffer(
        vkContext, bufferSize, vk::BufferUsageFlagBits::eUniformBuffer,
        vk::MemoryPropertyFlagBits::eHostVisible |
            vk::MemoryPropertyFlagBits::eHostCoherent);
    uniformBuffers.emplace_back(std::move(buffer));
    uniformBuffersMemory.emplace_back(std::move(bufferMem));
    uniformBuffersMapped.emplace_back(
        uniformBuffersMemory.back().mapMemory(0, bufferSize));
  }
}
