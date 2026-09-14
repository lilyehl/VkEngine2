#pragma once
#include "utils.hpp"
#include "vk_context.hpp"
#include <GLFW/glfw3.h>

class Swapchain {
public:
  Swapchain() = default;
  Swapchain(const VkContext &vkContext, GLFWwindow *window);

  vk::Extent2D get_extent() const { return swapChainExtent; }
  vk::SurfaceFormatKHR get_surface_format() const {
    return swapChainSurfaceFormat;
  }
  const std::vector<vk::raii::ImageView> &get_image_views() const {
    return swapChainImageViews;
  }
  const std::vector<vk::Image> &get_images() const { return swapChainImages; }
  const vk::raii::SwapchainKHR &get_swapchain() const { return swapChain; }
  const vk::raii::ImageView &get_depth_image_view() const {
    return depthImageView;
  }
  const vk::raii::Image &get_depth_image() const { return depthImage; }

  const vk::raii::ImageView &get_color_image_view() const {
    return colorImageView;
  }
  const vk::raii::Image &get_color_image() const { return colorImage; }

  void cleanup();
  void recreate_swapchain(const VkContext &vkContext, GLFWwindow *window);

private:
  vk::raii::SwapchainKHR swapChain = nullptr;
  std::vector<vk::Image> swapChainImages;
  std::vector<vk::raii::ImageView> swapChainImageViews; // how to access image

  vk::SurfaceFormatKHR swapChainSurfaceFormat;
  vk::Extent2D swapChainExtent; // image resolution

  // depth buffering
  vk::raii::Image depthImage = nullptr;
  vk::raii::DeviceMemory depthImageMemory = nullptr;
  vk::raii::ImageView depthImageView = nullptr;

  // multisampling
  vk::raii::Image colorImage = nullptr;
  vk::raii::DeviceMemory colorImageMemory = nullptr;
  vk::raii::ImageView colorImageView = nullptr;

  void create_swapchain(const VkContext &vkContext, GLFWwindow *window);
  void create_image_views(const VkContext &vkContext);
  void create_depth_resources(const VkContext &vkContext);
  void create_color_resources(const VkContext &vkContext);

  vk::raii::ImageView create_image_view(const VkContext &vkContext,
                                        vk::Image const &image,
                                        vk::Format format,
                                        vk::ImageAspectFlags aspectFlags);

  std::pair<vk::raii::Image, vk::raii::DeviceMemory>
  create_image(const VkContext &vkContext, uint32_t width, uint32_t height,
               vk::SampleCountFlagBits numSamples, vk::Format format,
               vk::ImageTiling tiling, vk::ImageUsageFlags usage,
               vk::MemoryPropertyFlags properties);
};
