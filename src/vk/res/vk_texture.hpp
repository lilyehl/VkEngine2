#pragma once

#include "utils.hpp"
#include "vk_context.hpp"

struct Texture {

  int width = 0;
  int height = 0;
  int channels = 0;

  vk::raii::Image image = nullptr;
  vk::raii::DeviceMemory imageMemory = nullptr;
  vk::raii::ImageView imageView = nullptr;
  vk::raii::Sampler sampler = nullptr;

  Texture() {}
  Texture(VkContext &vkContext, const char *filePath);

private:
  std::pair<vk::raii::Buffer, vk::raii::DeviceMemory>
  load_stb_to_buffer(VkContext &vkContext, const char *filePath);

  void create_texture_image(VkContext &vkContext,
                            const vk::raii::Buffer &stagingBuffer);

  void create_texture_image_view(VkContext &vkContext);

  void create_texture_sampler(VkContext &vkContext);
};
