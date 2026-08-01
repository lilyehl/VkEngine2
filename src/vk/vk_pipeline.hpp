#pragma once

#include "utils.hpp"
#include "vk_context.hpp"
#include "vk_mesh.hpp"
#include "vk_swapchain.hpp"

class PipelineBuilder {
public:
  vk::raii::Pipeline build(VkContext &vkContext, Swapchain &swapchain,
                           vk::raii::PipelineLayout &pipelineLayout);

private:
  // default topology set to triangle list without reuse
  // vk::PrimitiveTopology topology = vk::PrimitiveTopology::eTriangleList;
};
