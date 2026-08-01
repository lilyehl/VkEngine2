#pragma once

#include "camera.hpp"
// #include "triangle.hpp"
#include "vk/vk_renderer.hpp"
#include "window.hpp"

// #include <chrono>
#include <vector>

class App {
public:
  App() {
    window.init(&camera);
    renderer.init(window);
  }
  ~App() {}

  App(const App &) = delete;

  void run() {
    while (!window.should_close()) {
      window.update();
      renderer.draw(window, &camera);
    }
  }

private:
  Window window;
  Camera camera{glm::vec3(0.0f, 0.0f, 2.0f)};
  VkRenderer renderer;
  // Triangle triangle;
};
