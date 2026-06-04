#include "vk_engine.hpp"

#include "utils.hpp"
#include "vk_context.hpp"

const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;

class Example {
public:
  Example() {
    init_window();
    engine.init(window);
  }

  ~Example() {
    engine.wait_idle();
    glfwDestroyWindow(window);
    glfwTerminate();
  }

  void run() {
    while (!glfwWindowShouldClose(window)) {
      glfwPollEvents();
      engine.draw();
    }
  }

private:
  GLFWwindow *window = nullptr;
  VkEngine engine;

  void init_window() {
    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);
  }
};

int main() {
  try {
    Example app;
    app.run();
  } catch (const std::exception &e) {
    std::cerr << e.what() << std::endl;
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}
