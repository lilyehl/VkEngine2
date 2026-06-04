#pragma once

#include <utils.hpp>

const std::vector<char const *> validationLayers = {
    "VK_LAYER_KHRONOS_validation"};

#ifdef NDEBUG
constexpr bool enableValidationLayers = false;
#else
constexpr bool enableValidationLayers = true;
#endif

class VkContext {

private:
  vk::raii::Context context;
  vk::raii::Instance instance = nullptr;

  std::vector<const char *> requiredDeviceExtension = {
      vk::KHRSwapchainExtensionName};

  std::vector<const char *> get_required_instance_extensions();

  void create_instance();
  bool is_device_suitable(vk::raii::PhysicalDevice const &PhysicalDevice);
  void create_surface(GLFWwindow *window);
  void pick_physical_device();
  void create_logical_device();
  void create_command_pool();

public:
  vk::raii::Queue queue = nullptr;
  vk::raii::SurfaceKHR surface = nullptr;
  vk::raii::PhysicalDevice physicalDevice = nullptr;
  vk::raii::Device device = nullptr;
  uint32_t queueIndex = ~0;
  vk::raii::CommandPool commandPool = nullptr;

  VkContext() = default;
  VkContext(GLFWwindow *window);
};
