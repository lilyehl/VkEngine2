#pragma once

#include "res/vk_texture.hpp"
#include "res/vk_vertex.hpp"
#include "vk_context.hpp"
#include "vk_descriptors.hpp"
#include "vk_pipeline.hpp"
#include <vulkan/vulkan_raii.hpp>

class Logger {
public:
  Logger() {}
  ~Logger() {}

  const Texture &get_texture() { return texture; }
  vk::raii::Pipeline &get_pipeline() { return pipeline; }
  vk::raii::PipelineLayout &get_pipeline_layout() { return pipelineLayout; }

  void init(VkContext &vkContext);

  // CPU side: build the overlay geometry
  void clear() { vertices.clear(); }
  void add_quad(glm::vec2 min, glm::vec2 max, glm::vec2 uvMin, glm::vec2 uvMax,
                glm::vec3 color = glm::vec3(1.0f));

  // copies the CPU vertices into this frame's vertex buffer.
  // call after the frame's fence was waited on, before submitting
  void update(uint32_t frameIndex);

  void draw(const vk::raii::CommandBuffer &cmd, uint32_t frameIndex) const;

private:
  // enough for ~1000 characters (6 vertices per quad)
  static constexpr uint32_t MAX_VERTICES = 6 * 1024;

  Texture texture;

  Descriptors descriptors;
  vk::raii::PipelineLayout pipelineLayout = nullptr;
  vk::raii::Pipeline pipeline = nullptr;

  std::vector<Vertex2D> vertices;

  // one host-visible, persistently mapped buffer per frame in flight, so the
  // CPU never writes a buffer the GPU is still reading
  std::vector<vk::raii::Buffer> vertexBuffers;
  std::vector<vk::raii::DeviceMemory> vertexBuffersMemory;
  std::vector<void *> vertexBuffersMapped;

  void create_vertex_buffers(VkContext &vkContext);
};
