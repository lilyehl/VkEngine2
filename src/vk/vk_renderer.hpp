#pragma once

#include "vk_buffers.hpp"
#include "vk_context.hpp"
#include "vk_mesh.hpp"
#include "vk_pipeline.hpp"
#include "vk_swapchain.hpp"

#include "../window.hpp"

#include <memory>

struct UniformBufferObject {
  glm::mat4 model;
  glm::mat4 view;
  glm::mat4 proj;
};

class VkRenderer {
public:
  bool framebufferResized = false;
  void init(Window &window);
  void draw(Window &window, Camera *camera);
  void wait_idle(); // called before app destroys window

private:
  VkContext vkContext;
  Swapchain swapchain;
  // Camera camera;

  PipelineBuilder pipelineBuilder;
  vk::raii::Pipeline pipeline = nullptr;

  std::vector<vk::raii::CommandBuffer> commandBuffers;

  std::vector<vk::raii::Semaphore> presentCompleteSemaphores;
  std::vector<vk::raii::Semaphore> renderFinishedSemaphores;
  std::vector<vk::raii::Fence> inFlightFences;

  std::vector<vk::raii::Buffer> uniformBuffers;
  std::vector<vk::raii::DeviceMemory> uniformBuffersMemory;
  std::vector<void *> uniformBuffersMapped;

  uint32_t frameIndex = 0;

  // temp
  Voxel voxel;

  void create_sync_objects();

  void create_command_buffers();
  void record_command_buffer(uint32_t imageIndex);

  // not sure where to put these yet (a potential renderer ?)
  vk::raii::PipelineLayout pipelineLayout = nullptr;
  vk::raii::DescriptorSetLayout descriptorSetLayout = nullptr;
  vk::raii::DescriptorPool descriptorPool = nullptr;
  std::vector<vk::raii::DescriptorSet> descriptorSets;
  void create_descriptor_set_layout();
  void create_descriptor_pool();
  void create_descriptor_sets();

  void transition_image_layout(uint32_t imageIndex, vk::ImageLayout old_layout,
                               vk::ImageLayout new_layout,
                               vk::AccessFlags2 src_access_mask,
                               vk::AccessFlags2 dst_access_mask,
                               vk::PipelineStageFlags2 src_stage_mask,
                               vk::PipelineStageFlags2 dst_stage_mask);

  void updateUniformBuffer(uint32_t currentImage, Camera *camera); // TEMP
};
