#include "vk_image.hpp"

std::pair<vk::raii::Image, vk::raii::DeviceMemory>
Image::create_image(const VkContext &vkContext, uint32_t width, uint32_t height,
                    vk::SampleCountFlagBits numSamples, vk::Format format,
                    vk::ImageTiling tiling, vk::ImageUsageFlags usage,
                    vk::MemoryPropertyFlags properties) {
  vk::ImageCreateInfo imageInfo{.imageType = vk::ImageType::e2D,
                                .format = format,
                                .extent = {width, height, 1},
                                .mipLevels = 1,
                                .arrayLayers = 1,
                                .samples = numSamples,
                                .tiling = tiling,
                                .usage = usage,
                                .sharingMode = vk::SharingMode::eExclusive};

  vk::raii::Image image = vk::raii::Image(vkContext.device, imageInfo);

  vk::MemoryRequirements memRequirements = image.getMemoryRequirements();
  vk::MemoryAllocateInfo allocInfo{
      .allocationSize = memRequirements.size,
      .memoryTypeIndex = vkContext.find_memory_type(
          memRequirements.memoryTypeBits, properties)};
  vk::raii::DeviceMemory imageMemory =
      vk::raii::DeviceMemory(vkContext.device, allocInfo);
  image.bindMemory(imageMemory, 0);

  return {std::move(image), std::move(imageMemory)};
}

vk::raii::ImageView Image::create_image_view(const VkContext &vkContext,
                                             vk::Image const &image,
                                             vk::Format format,
                                             vk::ImageAspectFlags aspectFlags) {
  vk::ImageViewCreateInfo viewInfo{
      .image = image,
      .viewType = vk::ImageViewType::e2D,
      .format = format,
      .subresourceRange = {.aspectMask = aspectFlags,
                           .baseMipLevel = 0,
                           .levelCount = 1,
                           .baseArrayLayer = 0,
                           .layerCount = 1}};
  return vk::raii::ImageView(vkContext.device, viewInfo);
}

void Image::transition_image_layout(const vk::raii::CommandBuffer &cmd,
                                    vk::Image image, vk::ImageLayout oldLayout,
                                    vk::ImageLayout newLayout,
                                    vk::AccessFlags2 srcAccessMask,
                                    vk::AccessFlags2 dstAccessMask,
                                    vk::PipelineStageFlags2 srcStageMask,
                                    vk::PipelineStageFlags2 dstStageMask,
                                    vk::ImageAspectFlags aspectFlags) {
  vk::ImageMemoryBarrier2 barrier = {
      .srcStageMask = srcStageMask,
      .srcAccessMask = srcAccessMask,
      .dstStageMask = dstStageMask,
      .dstAccessMask = dstAccessMask,
      .oldLayout = oldLayout,
      .newLayout = newLayout,
      .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
      .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
      .image = image,
      .subresourceRange = {.aspectMask = aspectFlags,
                           .baseMipLevel = 0,
                           .levelCount = 1,
                           .baseArrayLayer = 0,
                           .layerCount = 1}};
  vk::DependencyInfo dependencyInfo = {.dependencyFlags = {},
                                       .imageMemoryBarrierCount = 1,
                                       .pImageMemoryBarriers = &barrier};
  cmd.pipelineBarrier2(dependencyInfo);
}

void Image::copy_buffer_to_image(const VkContext &vkContext,
                                 const vk::raii::Buffer &buffer,
                                 const vk::raii::Image &image, uint32_t width,
                                 uint32_t height) {
  auto cmd = vkContext.begin_single_time_commands();

  vk::BufferImageCopy region{
      .bufferOffset = 0,
      .bufferRowLength = 0,
      .bufferImageHeight = 0,
      .imageSubresource = {.aspectMask = vk::ImageAspectFlagBits::eColor,
                           .mipLevel = 0,
                           .baseArrayLayer = 0,
                           .layerCount = 1},
      .imageOffset = {0, 0, 0},
      .imageExtent = {width, height, 1}};

  cmd.copyBufferToImage(*buffer, *image, vk::ImageLayout::eTransferDstOptimal,
                        region);

  vkContext.end_single_time_commands(cmd);
}
