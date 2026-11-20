#pragma once

#include "utils.hpp"
#include "vk_context.hpp"

// Owns a descriptor set layout, a pool sized for it, and setCount sets.
//
// usage:
//   descriptors
//       .add_binding(0, vk::DescriptorType::eUniformBuffer,
//                    vk::ShaderStageFlagBits::eVertex)
//       .build(vkContext, MAX_FRAMES_IN_FLIGHT);
//   descriptors.write_buffer(vkContext, i, 0, uniformBuffers[i], size);
class Descriptors {
public:
  // 1. describe the layout (call before build)
  Descriptors &add_binding(uint32_t binding, vk::DescriptorType type,
                           vk::ShaderStageFlags stages);

  // 2. create layout + pool + setCount sets
  void build(const VkContext &vkContext, uint32_t setCount);

  // 3. point a set's binding at a resource
  void write_buffer(const VkContext &vkContext, uint32_t setIndex,
                    uint32_t binding, const vk::raii::Buffer &buffer,
                    vk::DeviceSize range);
  void write_image(const VkContext &vkContext, uint32_t setIndex,
                   uint32_t binding, const vk::raii::ImageView &view,
                   const vk::raii::Sampler &sampler);

  const vk::raii::DescriptorSetLayout &layout() const { return setLayout; }
  const vk::raii::DescriptorSet &set(uint32_t i) const { return sets[i]; }

private:
  std::vector<vk::DescriptorSetLayoutBinding> bindings;

  // members are destroyed in reverse order: sets must be freed before the
  // pool they were allocated from
  vk::raii::DescriptorSetLayout setLayout = nullptr;
  vk::raii::DescriptorPool pool = nullptr;
  std::vector<vk::raii::DescriptorSet> sets;

  vk::DescriptorType binding_type(uint32_t binding) const;
};
