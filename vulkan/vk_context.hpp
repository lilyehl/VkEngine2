#pragma once

#include <GLFW/glfw3.h>
#include <vulkan/vulkan_raii.hpp>

const std::vector<char const *> validationLayers = {
    "VK_LAYER_KHRONOS_validation"};

#ifdef NDEBUG
constexpr bool enableValidationLayers = false;
#else
constexpr bool enableValidationLayers = true;
#endif

class VkContext {
public:
  VkContext(GLFWwindow &window);

private:
  vk::raii::Context context;
  vk::raii::Instance instance = nullptr;
  vk::raii::PhysicalDevice physicalDevice = nullptr;

  std::vector<const char *> getRequiredInstanceExtensions();

  void create_instance();
  void pick_physical_device();
};
