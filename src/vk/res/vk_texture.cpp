#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "vk_buffers.hpp"
#include "vk_image.hpp"
#include "vk_texture.hpp"

Texture::Texture(VkContext &vkContext, const char *filePath) {

  // staging buffer only lives until the end of the constructor
  auto [stagingBuffer, stagingBufferMemory] =
      load_stb_to_buffer(vkContext, filePath);
  create_texture_image(vkContext, stagingBuffer);
  create_texture_image_view(vkContext);
  create_texture_sampler(vkContext);
}

std::pair<vk::raii::Buffer, vk::raii::DeviceMemory>
Texture::load_stb_to_buffer(VkContext &vkContext, const char *filePath) {
  // image to pixels
  stbi_uc *pixels =
      stbi_load(filePath, &width, &height, &channels, STBI_rgb_alpha);

  if (!pixels) {
    stbi_image_free(pixels);
    throw std::runtime_error("failed to load texture image!");
  }

  // pixels to staging buffer
  vk::DeviceSize imageSize = width * height * 4;
  auto [stagingBuffer, stagingBufferMemory] = Buffer::create_buffer(
      vkContext, imageSize, vk::BufferUsageFlagBits::eTransferSrc,
      // in gpu memory but visible to cpu
      vk::MemoryPropertyFlagBits::eHostVisible |
          // data immediately available (not cached)
          vk::MemoryPropertyFlagBits::eHostCoherent);

  void *data = stagingBufferMemory.mapMemory(0, imageSize);
  memcpy(data, pixels, imageSize);

  // clean up
  stagingBufferMemory.unmapMemory();
  stbi_image_free(pixels);

  return {std::move(stagingBuffer), std::move(stagingBufferMemory)};
}

void Texture::create_texture_image(VkContext &vkContext,
                                   const vk::raii::Buffer &stagingBuffer) {

  // create image
  std::tie(image, imageMemory) = Image::create_image(
      vkContext, width, height, vk::SampleCountFlagBits::e1,
      vk::Format::eR8G8B8A8Srgb, vk::ImageTiling::eOptimal,
      vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled,
      vk::MemoryPropertyFlagBits::eDeviceLocal);

  // undefined -> transfer dst, so the copy can write into it
  {
    auto cmd = vkContext.begin_single_time_commands();
    Image::transition_image_layout(cmd, *image, vk::ImageLayout::eUndefined,
                                   vk::ImageLayout::eTransferDstOptimal, {},
                                   vk::AccessFlagBits2::eTransferWrite,
                                   vk::PipelineStageFlagBits2::eTopOfPipe,
                                   vk::PipelineStageFlagBits2::eTransfer,
                                   vk::ImageAspectFlagBits::eColor);
    vkContext.end_single_time_commands(cmd);
  }

  Image::copy_buffer_to_image(vkContext, stagingBuffer, image, width, height);

  // transfer dst -> shader read only, so the fragment shader can access it
  {
    auto cmd = vkContext.begin_single_time_commands();
    Image::transition_image_layout(
        cmd, *image, vk::ImageLayout::eTransferDstOptimal,
        vk::ImageLayout::eShaderReadOnlyOptimal,
        vk::AccessFlagBits2::eTransferWrite, vk::AccessFlagBits2::eShaderRead,
        vk::PipelineStageFlagBits2::eTransfer,
        vk::PipelineStageFlagBits2::eFragmentShader,
        vk::ImageAspectFlagBits::eColor);

    vkContext.end_single_time_commands(cmd);
  }
}

void Texture::create_texture_image_view(VkContext &vkContext) {
  // format must match the one the image was created with
  imageView =
      Image::create_image_view(vkContext, *image, vk::Format::eR8G8B8A8Srgb,
                               vk::ImageAspectFlagBits::eColor);
}

void Texture::create_texture_sampler(VkContext &vkContext) {
  vk::PhysicalDeviceProperties properties =
      vkContext.physicalDevice.getProperties();

  vk::SamplerCreateInfo samplerInfo{
      // filtering when the texture is magnified/minified on screen
      .magFilter = vk::Filter::eLinear,
      .minFilter = vk::Filter::eLinear,
      .mipmapMode = vk::SamplerMipmapMode::eLinear,
      // what happens outside [0, 1] uv coordinates (i.e tile the texture)
      .addressModeU = vk::SamplerAddressMode::eRepeat,
      .addressModeV = vk::SamplerAddressMode::eRepeat,
      .addressModeW = vk::SamplerAddressMode::eRepeat,
      .mipLodBias = 0.0f,
      // sharper textures at grazing angles
      .anisotropyEnable = vk::True,
      .maxAnisotropy = properties.limits.maxSamplerAnisotropy,
      .compareEnable = vk::False, // only used for shadow maps
      .compareOp = vk::CompareOp::eAlways,
      .minLod = 0.0f,
      .maxLod = 0.0f, // no mipmaps yet
      .borderColor = vk::BorderColor::eIntOpaqueBlack,
      .unnormalizedCoordinates = vk::False}; // uv in [0, 1], not in texels

  sampler = vk::raii::Sampler(vkContext.device, samplerInfo);
}
