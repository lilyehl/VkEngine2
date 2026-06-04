#pragma once

#include "utils.hpp"
#include "vk_context.hpp"
#include "vk_swapchain.hpp"
#include "vulkan/vulkan.hpp"
#include <vulkan/vulkan_raii.hpp>

class PipelineBuilder {
public:
  vk::raii::Pipeline build(VkContext &vkContext, Swapchain &swapchain);

private:
  // default topology set to triangle list without reuse
  // vk::PrimitiveTopology topology = vk::PrimitiveTopology::eTriangleList;
  vk::raii::PipelineLayout pipelineLayout = nullptr;
};
