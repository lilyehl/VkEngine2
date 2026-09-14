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

  create_color_resources(vkContext);
  create_depth_resources(vkContext);
}

// image views
void Swapchain::create_image_views(const VkContext &vkContext) {

  assert(swapChainImageViews.empty());

  swapChainImageViews.reserve(swapChainImages.size());
  for (auto &image : swapChainImages) {
    swapChainImageViews.emplace_back(
        create_image_view(vkContext, image, swapChainSurfaceFormat.format,
                          vk::ImageAspectFlagBits::eColor));
  }
}

vk::raii::ImageView
Swapchain::create_image_view(VkContext const &vkContext, vk::Image const &image,
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

std::pair<vk::raii::Image, vk::raii::DeviceMemory>
Swapchain::create_image(const VkContext &vkContext, uint32_t width,
                        uint32_t height, vk::SampleCountFlagBits numSamples,
                        vk::Format format, vk::ImageTiling tiling,
                        vk::ImageUsageFlags usage,
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

void Swapchain::create_depth_resources(const VkContext &vkContext) {
  // assuming support for vk::Format::eD32Sfloat, could be checked for
  vk::Format depthFormat = vk::Format::eD32Sfloat;

  std::tie(depthImage, depthImageMemory) = create_image(
      vkContext, swapChainExtent.width, swapChainExtent.height,
      vkContext.msaaSamples, depthFormat, vk::ImageTiling::eOptimal,
      vk::ImageUsageFlagBits::eDepthStencilAttachment,
      vk::MemoryPropertyFlagBits::eDeviceLocal);
  depthImageView = create_image_view(vkContext, depthImage, depthFormat,
                                     vk::ImageAspectFlagBits::eDepth);
}

void Swapchain::create_color_resources(const VkContext &vkContext) {
  vk::Format colorFormat = swapChainSurfaceFormat.format;

  std::tie(colorImage, colorImageMemory) = create_image(
      vkContext, swapChainExtent.width, swapChainExtent.height,
      vkContext.msaaSamples, colorFormat, vk::ImageTiling::eOptimal,
      vk::ImageUsageFlagBits::eTransientAttachment |
          vk::ImageUsageFlagBits::eColorAttachment,
      vk::MemoryPropertyFlagBits::eDeviceLocal);
  colorImageView = create_image_view(vkContext, colorImage, colorFormat,
                                     vk::ImageAspectFlagBits::eColor);
}
