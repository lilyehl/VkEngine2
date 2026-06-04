#pragma once
#include "utils.hpp"
#include "vk_context.hpp"
#include "vulkan/vulkan.hpp"
#include <GLFW/glfw3.h>
#include <vulkan/vulkan_raii.hpp>

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

private:
  vk::raii::SwapchainKHR swapChain = nullptr;
  std::vector<vk::Image> swapChainImages;
  std::vector<vk::raii::ImageView> swapChainImageViews; // how to access image

  vk::SurfaceFormatKHR swapChainSurfaceFormat;
  vk::Extent2D swapChainExtent; // image resolution

  void create_swapchain(const VkContext &vkContext, GLFWwindow *window);
  void create_image_views(const VkContext &vkContext);
};
