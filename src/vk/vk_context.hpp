#pragma once

#include "utils.hpp"

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
  vk::SampleCountFlagBits
  get_max_usable_sample_count(); // for multisample antialiasing

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
  vk::SampleCountFlagBits msaaSamples = vk::SampleCountFlagBits::e1;

  VkContext() = default;
  VkContext(GLFWwindow *window);

  // buffer allocation helper
  uint32_t find_memory_type(uint32_t typeFilter,
                            vk::MemoryPropertyFlags properties) const;
  vk::raii::CommandBuffer begin_single_time_commands() const;
  void end_single_time_commands(vk::raii::CommandBuffer &commandBuffer) const;
};
