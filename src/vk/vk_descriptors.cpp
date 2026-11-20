#include "vk_descriptors.hpp"

#include <map>

Descriptors &Descriptors::add_binding(uint32_t binding, vk::DescriptorType type,
                                      vk::ShaderStageFlags stages) {
  bindings.push_back({.binding = binding,
                      .descriptorType = type,
                      .descriptorCount = 1,
                      .stageFlags = stages});
  return *this;
}

void Descriptors::build(const VkContext &vkContext, uint32_t setCount) {
  assert(!bindings.empty() && setCount > 0);

  // layout
  vk::DescriptorSetLayoutCreateInfo layoutInfo{
      .bindingCount = static_cast<uint32_t>(bindings.size()),
      .pBindings = bindings.data()};
  setLayout = vk::raii::DescriptorSetLayout(vkContext.device, layoutInfo);

  // pool : enough descriptors of each type for every set
  std::map<vk::DescriptorType, uint32_t> countPerType;
  for (const auto &b : bindings) {
    countPerType[b.descriptorType] += b.descriptorCount;
  }
  std::vector<vk::DescriptorPoolSize> poolSizes;
  for (auto [type, count] : countPerType) {
    poolSizes.push_back({.type = type, .descriptorCount = count * setCount});
  }

  vk::DescriptorPoolCreateInfo poolInfo{
      // vk::raii::DescriptorSet frees itself on destruction, which requires
      // this flag
      .flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet,
      .maxSets = setCount,
      .poolSizeCount = static_cast<uint32_t>(poolSizes.size()),
      .pPoolSizes = poolSizes.data()};
  pool = vk::raii::DescriptorPool(vkContext.device, poolInfo);

  // sets : all share the same layout
  std::vector<vk::DescriptorSetLayout> layouts(setCount, *setLayout);
  vk::DescriptorSetAllocateInfo allocInfo{.descriptorPool = pool,
                                          .descriptorSetCount = setCount,
                                          .pSetLayouts = layouts.data()};
  sets = vkContext.device.allocateDescriptorSets(allocInfo);
}

void Descriptors::write_buffer(const VkContext &vkContext, uint32_t setIndex,
                               uint32_t binding, const vk::raii::Buffer &buffer,
                               vk::DeviceSize range) {
  vk::DescriptorBufferInfo bufferInfo{
      .buffer = buffer, .offset = 0, .range = range};

  vk::WriteDescriptorSet write{.dstSet = sets[setIndex],
                               .dstBinding = binding,
                               .dstArrayElement = 0,
                               .descriptorCount = 1,
                               .descriptorType = binding_type(binding),
                               .pBufferInfo = &bufferInfo};
  vkContext.device.updateDescriptorSets(write, {});
}

void Descriptors::write_image(const VkContext &vkContext, uint32_t setIndex,
                              uint32_t binding, const vk::raii::ImageView &view,
                              const vk::raii::Sampler &sampler) {
  vk::DescriptorImageInfo imageInfo{
      .sampler = sampler,
      .imageView = view,
      .imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal};

  vk::WriteDescriptorSet write{.dstSet = sets[setIndex],
                               .dstBinding = binding,
                               .dstArrayElement = 0,
                               .descriptorCount = 1,
                               .descriptorType = binding_type(binding),
                               .pImageInfo = &imageInfo};
  vkContext.device.updateDescriptorSets(write, {});
}

// the type a write must use is the one the binding was declared with
vk::DescriptorType Descriptors::binding_type(uint32_t binding) const {
  for (const auto &b : bindings) {
    if (b.binding == binding) {
      return b.descriptorType;
    }
  }
  throw std::runtime_error("Descriptors: no binding " +
                           std::to_string(binding));
}
