#include "vk_renderer.hpp"
#include "vk_image.hpp"
#include "vk_pipeline.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <chrono>

#include <iostream>

void VkRenderer::init(Window &window, MeshData &voxelData) {
  LOG("Initializing vulkan context...")
  vkContext = VkContext(window.window);

  LOG("Creating a swapchain...")
  swapchain = Swapchain(vkContext, window.window);

  LOG("Initializing logger...");
  logger.init(vkContext);

  LOG("Creating logger pipeline..")
  // overlay a test 2d texture on screen
  logger.get_pipeline() =
      PipelineBuilder()
          .set_shader("shaders/texture.spv")
          .set_vertex_input(Vertex2D::description())
          .set_depth_test(false, false)
          .set_cull_mode(vk::CullModeFlagBits::eNone)
          .set_blending(true)
          .build(vkContext, swapchain, logger.get_pipeline_layout());

  // TODO : move this to the terrain class ?
  LOG("Creating descriptors..");
  descriptors
      .add_binding(0, vk::DescriptorType::eUniformBuffer,
                   vk::ShaderStageFlagBits::eVertex)

      .build(vkContext, MAX_FRAMES_IN_FLIGHT);

  vk::PipelineLayoutCreateInfo pipelineLayoutInfo{.setLayoutCount = 1,
                                                  .pSetLayouts =
                                                      &*descriptors.layout(),
                                                  .pushConstantRangeCount = 0};

  pipelineLayout =
      vk::raii::PipelineLayout(vkContext.device, pipelineLayoutInfo);

  LOG("Creating graphics pipeline..")
  // 3d voxels (depth on, back-face culling, no blend)
  pipeline = PipelineBuilder()
                 .set_shader("shaders/voxel.spv")
                 .set_vertex_input(Vertex::description())
                 .build(vkContext, swapchain, pipelineLayout);

  Buffer::create_uniform_buffers(vkContext, sizeof(UniformBufferObject),
                                 uniformBuffers, uniformBuffersMemory,
                                 uniformBuffersMapped);

  LOG("Recording voxel mesh")
  voxelMesh = Mesh(vkContext, voxelData);

  // needs the uniform buffers to exist
  write_descriptor_sets();

  LOG("Initializing sync objects and command buffer...")
  create_command_buffers();
  create_sync_objects();

  LOG("\nVulkan engine initialized !");
}

void VkRenderer::wait_idle() {
  // called before app teardown
  vkContext.device.waitIdle();
}

void VkRenderer::draw(Window &window, Camera *camera) {

  // wait for GPU to finish w/ the semaphore and frame buffer
  auto fenceResult = vkContext.device.waitForFences(*inFlightFences[frameIndex],
                                                    vk::True, UINT64_MAX);
  if (fenceResult != vk::Result::eSuccess) {
    throw std::runtime_error("failed to wait for fence!");
  }

  // get next available image in swapchain, give it to present semaphore
  auto [result, imageIndex] = swapchain.get_swapchain().acquireNextImage(
      UINT64_MAX, *acquireSemaphores[frameIndex], nullptr);

  if ((result == vk::Result::eSuboptimalKHR) ||
      (result == vk::Result::eErrorOutOfDateKHR)) {
    framebufferResized = false;
    swapchain.recreate_swapchain(vkContext, window.window);
    return;
  }

  if (result != vk::Result::eSuccess && result != vk::Result::eSuboptimalKHR) {
    assert(result == vk::Result::eTimeout || result == vk::Result::eNotReady);
    throw std::runtime_error("failed to acquire swap chain image!");
  }

  // if all goes well, we are submitting during this frame
  // reset fences and command buffer
  vkContext.device.resetFences(*inFlightFences[frameIndex]);
  commandBuffers[frameIndex].reset();

  record_command_buffer(imageIndex);

  updateUniformBuffer(frameIndex, camera);
  logger.update(frameIndex);

  // submit command buffer to graphics queue
  vk::PipelineStageFlags waitDestinationStageMask(
      vk::PipelineStageFlagBits::eColorAttachmentOutput);

  const vk::SubmitInfo submitInfo{
      .waitSemaphoreCount = 1,
      .pWaitSemaphores = &*acquireSemaphores[frameIndex],
      .pWaitDstStageMask = &waitDestinationStageMask,
      .commandBufferCount = 1,
      .pCommandBuffers = &*commandBuffers[frameIndex],
      .signalSemaphoreCount = 1,
      .pSignalSemaphores = &*submitSemaphores[imageIndex]};
  vkContext.queue.submit(submitInfo, *inFlightFences[frameIndex]);

  // present rendered info
  const vk::PresentInfoKHR presentInfoKHR{
      .waitSemaphoreCount = 1,
      .pWaitSemaphores = &*submitSemaphores[imageIndex],
      .swapchainCount = 1,
      .pSwapchains = &*swapchain.get_swapchain(),
      .pImageIndices = &imageIndex};

  result = vkContext.queue.presentKHR(presentInfoKHR);
  if ((result == vk::Result::eSuboptimalKHR) ||
      (result == vk::Result::eErrorOutOfDateKHR) || framebufferResized) {
    framebufferResized = false;
    swapchain.recreate_swapchain(vkContext, window.window);
  } else {
    // There are no other success codes than eSuccess; on any error code,
    // presentKHR already threw an exception.
    assert(result == vk::Result::eSuccess);
  }

  // move to next next frame in flight
  frameIndex = (frameIndex + 1) % MAX_FRAMES_IN_FLIGHT;
}

void VkRenderer::create_command_buffers() {
  vk::CommandBufferAllocateInfo allocInfo{
      .commandPool = vkContext.commandPool,
      .level = vk::CommandBufferLevel::ePrimary,
      .commandBufferCount = MAX_FRAMES_IN_FLIGHT};
  commandBuffers = vk::raii::CommandBuffers(vkContext.device, allocInfo);
}

void VkRenderer::record_command_buffer(uint32_t imageIndex) {
  auto &commandBuffer = commandBuffers[frameIndex];
  commandBuffer.begin({});

  // Before starting rendering, transition the swapchain image to
  // vk::ImageLayout::eColorAttachmentOptimal
  Image::transition_image_layout(
      commandBuffer, swapchain.get_images()[imageIndex],
      vk::ImageLayout::eUndefined, vk::ImageLayout::eColorAttachmentOptimal,
      {}, // srcAccessMask (no need to wait for previous operations)
      vk::AccessFlagBits2::eColorAttachmentWrite,         // dstAccessMask
      vk::PipelineStageFlagBits2::eColorAttachmentOutput, // srcStage
      vk::PipelineStageFlagBits2::eColorAttachmentOutput, // dstStage
      vk::ImageAspectFlagBits::eColor);

  // Transition the multisampled color image to
  // vk::ImageLayout::eColorAttachmentOptimal
  Image::transition_image_layout(
      commandBuffer, *(swapchain.get_color_image()),
      vk::ImageLayout::eUndefined, vk::ImageLayout::eColorAttachmentOptimal,
      vk::AccessFlagBits2::eColorAttachmentWrite,
      vk::AccessFlagBits2::eColorAttachmentWrite,
      vk::PipelineStageFlagBits2::eColorAttachmentOutput,
      vk::PipelineStageFlagBits2::eColorAttachmentOutput,
      vk::ImageAspectFlagBits::eColor);

  // Transition depth image to depth attachment optimal layout
  Image::transition_image_layout(
      commandBuffer, *(swapchain.get_depth_image()),
      vk::ImageLayout::eUndefined, vk::ImageLayout::eDepthAttachmentOptimal,
      vk::AccessFlagBits2::eDepthStencilAttachmentWrite,
      vk::AccessFlagBits2::eDepthStencilAttachmentWrite,
      vk::PipelineStageFlagBits2::eEarlyFragmentTests |
          vk::PipelineStageFlagBits2::eLateFragmentTests,
      vk::PipelineStageFlagBits2::eEarlyFragmentTests |
          vk::PipelineStageFlagBits2::eLateFragmentTests,
      vk::ImageAspectFlagBits::eDepth);

  // color attachment (multisampled) with resolve attachment
  vk::ClearValue clearColor = vk::ClearColorValue(0.0f, 0.0f, 0.0f, 1.0f);
  vk::RenderingAttachmentInfo colorAttachment = {
      .imageView = swapchain.get_color_image_view(),
      .imageLayout = vk::ImageLayout::eColorAttachmentOptimal,
      .resolveMode = vk::ResolveModeFlagBits::eAverage,
      .resolveImageView = swapchain.get_image_views()[imageIndex],
      .resolveImageLayout = vk::ImageLayout::eColorAttachmentOptimal,
      .loadOp = vk::AttachmentLoadOp::eClear,
      .storeOp = vk::AttachmentStoreOp::eStore,
      .clearValue = clearColor};

  // depth buffering
  vk::ClearValue clearDepth = vk::ClearDepthStencilValue(1.0f, 0);
  vk::RenderingAttachmentInfo depthAttachment = {
      .imageView = swapchain.get_depth_image_view(),
      .imageLayout = vk::ImageLayout::eDepthAttachmentOptimal,
      .loadOp = vk::AttachmentLoadOp::eClear,
      .storeOp = vk::AttachmentStoreOp::eDontCare,
      .clearValue = clearDepth};

  // rendering info
  auto extent = swapchain.get_extent();
  vk::RenderingInfo renderingInfo = {
      .renderArea = {.offset = {0, 0}, .extent = extent},
      .layerCount = 1,
      .colorAttachmentCount = 1,
      .pColorAttachments = &colorAttachment,
      .pDepthAttachment = &depthAttachment};

  commandBuffer.beginRendering(renderingInfo);
  commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, *pipeline);

  // viewport and scissor state
  commandBuffer.setViewport(
      0, vk::Viewport(0.0f, 0.0f, static_cast<float>(extent.width),
                      static_cast<float>(extent.height), 0.0f, 1.0f));
  commandBuffer.setScissor(0, vk::Rect2D(vk::Offset2D(0, 0), extent));

  // DRAW !!!!

  commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, *pipeline);

  commandBuffer.bindVertexBuffers(0, *voxelMesh.vertexBuffer, {0});
  commandBuffer.bindIndexBuffer(
      *voxelMesh.indexBuffer, 0,
      vk::IndexTypeValue<decltype(MeshData::indices)::value_type>::value);

  commandBuffers[frameIndex].bindDescriptorSets(
      vk::PipelineBindPoint::eGraphics, pipelineLayout, 0,
      *descriptors.set(frameIndex), nullptr);
  commandBuffer.drawIndexed(
      static_cast<uint32_t>(voxelMesh.meshData->indices.size()), 1, 0, 0, 0);

  // overlay last, on top of the scene (its pipeline ignores depth)
  logger.draw(commandBuffer, frameIndex);

  // clean uo
  commandBuffer.endRendering();

  // After rendering, transition the swapchain image to
  // vk::ImageLayout::ePresentSrcKHR
  Image::transition_image_layout(
      commandBuffer, swapchain.get_images()[imageIndex],
      vk::ImageLayout::eColorAttachmentOptimal, vk::ImageLayout::ePresentSrcKHR,
      vk::AccessFlagBits2::eColorAttachmentWrite,         // srcAccessMask
      {},                                                 // dstAccessMask
      vk::PipelineStageFlagBits2::eColorAttachmentOutput, // srcStage
      vk::PipelineStageFlagBits2::eBottomOfPipe,          // dstStage
      vk::ImageAspectFlagBits::eColor);

  commandBuffer.end();
}

void VkRenderer::create_sync_objects() {
  assert(acquireSemaphores.empty() && submitSemaphores.empty() &&
         inFlightFences.empty());

  for (size_t i = 0; i < swapchain.get_images().size(); i++) {
    submitSemaphores.emplace_back(vkContext.device, vk::SemaphoreCreateInfo());
  }

  for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
    acquireSemaphores.emplace_back(vkContext.device, vk::SemaphoreCreateInfo());
    inFlightFences.emplace_back(
        vkContext.device,
        vk::FenceCreateInfo{.flags = vk::FenceCreateFlagBits::eSignaled});
  }
}

void VkRenderer::write_descriptor_sets() {
  // terrain only has the ubo; the logger writes its own texture descriptor
  for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
    descriptors.write_buffer(vkContext, i, 0, uniformBuffers[i],
                             sizeof(UniformBufferObject));
  }
}

void VkRenderer::updateUniformBuffer(uint32_t currentImage, Camera *camera) {
  static auto startTime = std::chrono::high_resolution_clock::now();

  auto currentTime = std::chrono::high_resolution_clock::now();
  float time = std::chrono::duration<float>(currentTime - startTime).count();

  UniformBufferObject ubo{};
  ubo.model =
      glm::mat4(1.0f); // rotate(glm::mat4(1.0f), time * glm::radians(90.0f),
                       //        glm::vec3(0.0f, 0.0f, 1.0f));
  // ubo.view = lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f),
  //                   glm::vec3(0.0f, 0.0f, 1.0f));
  // ubo.proj =
  //     glm::perspective(glm::radians(45.0f),
  //                      static_cast<float>(swapchain.get_extent().width) /
  //                          static_cast<float>(swapchain.get_extent().height),
  //                      0.1f, 10.0f);
  // ubo.proj[1][1] *= -1;
  ubo.proj = camera->Mprojection;
  ubo.view = camera->Mview;

  memcpy(uniformBuffersMapped[currentImage], &ubo, sizeof(ubo));
}
