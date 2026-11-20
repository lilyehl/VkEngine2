#include "vk_logger.hpp"
#include "vk_buffers.hpp"

void Logger::init(VkContext &vkContext) {

  texture = Texture(vkContext, "textures/rocket.png");

  descriptors
      .add_binding(0, vk::DescriptorType::eCombinedImageSampler,
                   vk::ShaderStageFlagBits::eFragment)
      .build(vkContext, 1);
  descriptors.write_image(vkContext, 0, 0, texture.imageView, texture.sampler);

  vk::PipelineLayoutCreateInfo pipelineLayoutInfo{.setLayoutCount = 1,
                                                  .pSetLayouts =
                                                      &*descriptors.layout(),
                                                  .pushConstantRangeCount = 0};

  pipelineLayout =
      vk::raii::PipelineLayout(vkContext.device, pipelineLayoutInfo);

  create_vertex_buffers(vkContext);

  // TEMP test: the whole texture in the top-left corner of the screen
  add_quad({-0.95f, -0.95f}, {-0.45f, -0.45f}, {0.0f, 0.0f}, {1.0f, 1.0f});
}

void Logger::create_vertex_buffers(VkContext &vkContext) {
  vk::DeviceSize bufferSize = sizeof(Vertex2D) * MAX_VERTICES;

  for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
    auto [buffer, bufferMemory] = Buffer::create_buffer(
        vkContext, bufferSize, vk::BufferUsageFlagBits::eVertexBuffer,
        // written by the CPU every frame, no staging needed
        vk::MemoryPropertyFlagBits::eHostVisible |
            vk::MemoryPropertyFlagBits::eHostCoherent);

    vertexBuffers.emplace_back(std::move(buffer));
    vertexBuffersMemory.emplace_back(std::move(bufferMemory));

    // stays mapped for the buffer's whole lifetime
    vertexBuffersMapped.emplace_back(
        vertexBuffersMemory.back().mapMemory(0, bufferSize));
  }
}

void Logger::add_quad(glm::vec2 min, glm::vec2 max, glm::vec2 uvMin,
                      glm::vec2 uvMax, glm::vec3 color) {
  // two triangles, no index buffer
  Vertex2D topLeft{{min.x, min.y}, color, {uvMin.x, uvMin.y}};
  Vertex2D topRight{{max.x, min.y}, color, {uvMax.x, uvMin.y}};
  Vertex2D bottomRight{{max.x, max.y}, color, {uvMax.x, uvMax.y}};
  Vertex2D bottomLeft{{min.x, max.y}, color, {uvMin.x, uvMax.y}};

  vertices.insert(vertices.end(), {topLeft, topRight, bottomRight, topLeft,
                                   bottomRight, bottomLeft});
}

void Logger::update(uint32_t frameIndex) {
  if (vertices.size() > MAX_VERTICES) {
    LOG("Logger: too many vertices, overlay truncated");
  }
  size_t count = std::min<size_t>(vertices.size(), MAX_VERTICES);

  memcpy(vertexBuffersMapped[frameIndex], vertices.data(),
         count * sizeof(Vertex2D));
}

void Logger::draw(const vk::raii::CommandBuffer &cmd,
                  uint32_t frameIndex) const {
  uint32_t count =
      static_cast<uint32_t>(std::min<size_t>(vertices.size(), MAX_VERTICES));
  if (count == 0) {
    return;
  }

  cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, *pipeline);
  cmd.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, *pipelineLayout, 0,
                         *descriptors.set(0), nullptr);
  cmd.bindVertexBuffers(0, *vertexBuffers[frameIndex], {0});
  cmd.draw(count, 1, 0, 0);
}
