#include "vk_swapchain.hpp"
#include "vk_context.hpp"
#include <algorithm>
#include <cstdint>
#include <limits>

// static helper functions
static vk::SurfaceFormatKHR choose_swap_surface_format(
    const std::vector<vk::SurfaceFormatKHR> &availableFormats) {
  const auto formatIt =
      std::ranges::find_if(availableFormats, [](const auto &format) {
        return format.format == vk::Format::eB8G8R8A8Srgb &&
               format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear;
      });
  return formatIt != availableFormats.end() ? *formatIt : availableFormats[0];
}

static vk::PresentModeKHR choose_swap_present_mode(
    std::vector<vk::PresentModeKHR> const &availablePresentModes) {
  assert(std::ranges::any_of(availablePresentModes, [](auto presentMode) {
    return presentMode == vk::PresentModeKHR::eFifo;
  }));
  return std::ranges::any_of(availablePresentModes,
                             [](const vk::PresentModeKHR value) {
                               return vk::PresentModeKHR::eMailbox == value;
                             })
             ? vk::PresentModeKHR::eMailbox
             : vk::PresentModeKHR::eFifo;
}

static vk::Extent2D
choose_swap_extent(vk::SurfaceCapabilitiesKHR const &capabilities,
                   GLFWwindow *window) {
  if (capabilities.currentExtent.width !=
      std::numeric_limits<uint32_t>::max()) {
    return capabilities.currentExtent;
  }
  int width, height;
  glfwGetFramebufferSize(window, &width, &height);

  return {std::clamp<uint32_t>(width, capabilities.minImageExtent.width,
                               capabilities.maxImageExtent.width),
          std::clamp<uint32_t>(height, capabilities.minImageExtent.height,
                               capabilities.maxImageExtent.height)};
}

static uint32_t choose_swap_min_image_count(
    vk::SurfaceCapabilitiesKHR const &surfaceCapabilities) {
  auto minImageCount = std::max(3u, surfaceCapabilities.minImageCount);
  if ((0 < surfaceCapabilities.maxImageCount) &&
      (surfaceCapabilities.maxImageCount < minImageCount)) {
    minImageCount = surfaceCapabilities.maxImageCount;
  }
  return minImageCount;
}

// swapchain
Swapchain::Swapchain(const VkContext &vkContext, GLFWwindow *window) {
  create_swapchain(vkContext, window);
  create_image_views(vkContext);
}

void Swapchain::create_swapchain(const VkContext &vkContext,
                                 GLFWwindow *window) {
  vk::SurfaceCapabilitiesKHR surfaceCapabilities =
      vkContext.physicalDevice.getSurfaceCapabilitiesKHR(*(vkContext.surface));
  swapChainExtent = choose_swap_extent(surfaceCapabilities, window);
  uint32_t minImageCount = choose_swap_min_image_count(surfaceCapabilities);

  std::vector<vk::SurfaceFormatKHR> availableFormats =
      vkContext.physicalDevice.getSurfaceFormatsKHR(*(vkContext.surface));
  swapChainSurfaceFormat = choose_swap_surface_format(availableFormats);

  std::vector<vk::PresentModeKHR> availablePresentModes =
      vkContext.physicalDevice.getSurfacePresentModesKHR(*(vkContext.surface));
  vk::PresentModeKHR presentMode =
      choose_swap_present_mode(availablePresentModes);

  vk::SwapchainCreateInfoKHR swapChainCreateInfo{
      .surface = *(vkContext.surface),
      .minImageCount = minImageCount,
      .imageFormat = swapChainSurfaceFormat.format,
      .imageColorSpace = swapChainSurfaceFormat.colorSpace,
      .imageExtent = swapChainExtent,
      .imageArrayLayers = 1,
      .imageUsage = vk::ImageUsageFlagBits::eColorAttachment,
      .imageSharingMode = vk::SharingMode::eExclusive,
      .preTransform = surfaceCapabilities.currentTransform,
      .compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque,
      .presentMode = presentMode,
      .clipped = true};

  swapChain = vk::raii::SwapchainKHR(vkContext.device, swapChainCreateInfo);
  swapChainImages = swapChain.getImages();
}

// image views
void Swapchain::create_image_views(const VkContext &vkContext) {
  assert(swapChainImageViews.empty());

  vk::ImageViewCreateInfo imageViewCreateInfo{
      .viewType = vk::ImageViewType::e2D,
      .format = swapChainSurfaceFormat.format,
      .subresourceRange = {vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1}};
  for (auto &image : swapChainImages) {
    imageViewCreateInfo.image = image;
    swapChainImageViews.emplace_back(vkContext.device, imageViewCreateInfo);
  }
}

void Swapchain::recreate_swapchain(const VkContext &vkContext,
                                   GLFWwindow *window) {
  int width = 0, height = 0;
  glfwGetFramebufferSize(window, &width, &height);
  while (width == 0 || height == 0) {
    glfwGetFramebufferSize(window, &width, &height);
    glfwWaitEvents();
  }
  vkContext.device.waitIdle();

  cleanup();

  create_swapchain(vkContext, window);
  create_image_views(vkContext);
}

void Swapchain::cleanup() {
  swapChainImageViews.clear();
  swapChain = nullptr;
}
