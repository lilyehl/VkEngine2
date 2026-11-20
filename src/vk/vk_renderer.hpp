#pragma once

#include "res/vk_mesh.hpp"
#include "vk_buffers.hpp"
#include "vk_context.hpp"
#include "vk_descriptors.hpp"
#include "vk_logger.hpp"
#include "vk_pipeline.hpp"
#include "vk_swapchain.hpp"

#include "../window.hpp"

#include <cstdint>
#include <memory>

struct UniformBufferObject {
  glm::mat4 model;
  glm::mat4 view;
  glm::mat4 proj;
};

class VkRenderer {
public:
  bool framebufferResized = false;

  void init(Window &window, MeshData &voxelData);
  void draw(Window &window, Camera *camera);
  void update_logger(Window &Window);

  void wait_idle(); // called before app destroys window

private:
  VkContext vkContext;
  Swapchain swapchain;

  Logger logger;

  vk::raii::Pipeline pipeline = nullptr;
  vk::raii::PipelineLayout pipelineLayout = nullptr;

  Descriptors descriptors; // one set per frame in flight

  std::vector<vk::raii::CommandBuffer> commandBuffers;

  // Sync objects
  std::vector<vk::raii::Semaphore> acquireSemaphores; // present complete
  std::vector<vk::raii::Semaphore> submitSemaphores;  // render finished
  std::vector<vk::raii::Fence> inFlightFences;

  // Uniform buffer
  std::vector<vk::raii::Buffer> uniformBuffers;
  std::vector<vk::raii::DeviceMemory> uniformBuffersMemory;
  std::vector<void *> uniformBuffersMapped;

  uint32_t frameIndex = 0;

  // currently mesh = terrain
  Mesh voxelMesh;

  void create_sync_objects();

  void create_command_buffers();
  void record_command_buffer(uint32_t imageIndex);

  void write_descriptor_sets();

  void updateUniformBuffer(uint32_t currentImage, Camera *camera); // TEMP
};
