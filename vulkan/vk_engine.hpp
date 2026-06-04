#pragma once

#include "vk_context.hpp"
#include "vk_pipeline.hpp"
#include "vk_swapchain.hpp"

#include <memory>

class VkEngine {
public:
  void init(GLFWwindow *window);
  void draw();
  void wait_idle(); // called before app destroys window

private:
  VkContext vkContext;
  Swapchain swapchain;

  PipelineBuilder pipelineBuilder;
  vk::raii::Pipeline pipeline = nullptr;

  vk::raii::CommandBuffer commandBuffer = nullptr;

  vk::raii::Semaphore presentCompleteSemaphore = nullptr;
  vk::raii::Semaphore renderFinishedSemaphore = nullptr;
  vk::raii::Fence drawFence = nullptr;

  void create_command_buffer();
  void create_sync_objects();

  void record_command_buffer(uint32_t imageIndex);

  void transition_image_layout(uint32_t imageIndex, vk::ImageLayout old_layout,
                               vk::ImageLayout new_layout,
                               vk::AccessFlags2 src_access_mask,
                               vk::AccessFlags2 dst_access_mask,
                               vk::PipelineStageFlags2 src_stage_mask,
                               vk::PipelineStageFlags2 dst_stage_mask);
};
