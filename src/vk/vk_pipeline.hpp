#pragma once

#include "res/vk_vertex.hpp"
#include "utils.hpp"
#include "vk_context.hpp"
#include "vk_swapchain.hpp"

// Builds a graphics pipeline for dynamic rendering into the swapchain's
// targets (MSAA color + depth)
// Defaults match an opaque 3D mesh:
// depth test + write on, back-face culling, no blending.
//
class PipelineBuilder {
public:
  vk::raii::Pipeline build(VkContext &vkContext, Swapchain &swapchain,
                           vk::raii::PipelineLayout &pipelineLayout);

  // .spv file with a vertMain and a fragMain entry point
  PipelineBuilder &set_shader(std::string shaderPath);

  // vertex buffer layout; leave unset for pipelines without vertex buffers
  PipelineBuilder &set_vertex_input(VertexInputDescription vertexInput);

  PipelineBuilder &set_depth_test(bool testEnable, bool writeEnable);

  PipelineBuilder &set_cull_mode(vk::CullModeFlags cullMode);

  // standard alpha blending: src * alpha + dst * (1 - alpha)
  PipelineBuilder &set_blending(bool enable);

private:
  std::string shaderPath;
  VertexInputDescription vertexInput{};

  bool depthTestEnable = true;
  bool depthWriteEnable = true;
  vk::CullModeFlags cullMode = vk::CullModeFlagBits::eBack;
  bool blendEnable = false;
};
