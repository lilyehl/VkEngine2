#pragma once

#include "utils.hpp"
#include "vk_context.hpp"

namespace Image {

std::pair<vk::raii::Image, vk::raii::DeviceMemory>
create_image(const VkContext &vkContext, uint32_t width, uint32_t height,
             vk::SampleCountFlagBits numSamples, vk::Format format,
             vk::ImageTiling tiling, vk::ImageUsageFlags usage,
             vk::MemoryPropertyFlags properties);

vk::raii::ImageView create_image_view(const VkContext &vkContext,
                                      vk::Image const &image, vk::Format format,
                                      vk::ImageAspectFlags aspectFlags);

// records a layout transition barrier into command buffer
void transition_image_layout(const vk::raii::CommandBuffer &cmd,
                             vk::Image image, vk::ImageLayout oldLayout,
                             vk::ImageLayout newLayout,
                             vk::AccessFlags2 srcAccessMask,
                             vk::AccessFlags2 dstAccessMask,
                             vk::PipelineStageFlags2 srcStageMask,
                             vk::PipelineStageFlags2 dstStageMask,
                             vk::ImageAspectFlags aspectFlags);

// image must already be in vk::ImageLayout::eTransferDstOptimal
void copy_buffer_to_image(const VkContext &vkContext,
                          const vk::raii::Buffer &buffer,
                          const vk::raii::Image &image, uint32_t width,
                          uint32_t height);

} // namespace Image
