#pragma once

#include "camera.hpp"
#include "terrain.hpp"
#include "vk/vk_renderer.hpp"
#include "window.hpp"

class App {
public:
  App() {
    window.init(&camera);
    terrain.generate();
    renderer.init(window, terrain.voxels);
  }
  ~App() {}

  App(const App &) = delete;

  void run() {
    while (!window.should_close()) {
      window.update();
      renderer.draw(window, &camera);
    }
    renderer.wait_idle();
  }

private:
  Window window;
  Camera camera{glm::vec3(5.0f, 5.0f, 35.0f)};
  VkRenderer renderer;
  Terrain terrain;
};
