#include "vk_pipeline.hpp"
#include "fstream"

// helper function
static vk::raii::ShaderModule
load_shader_module(const std::string &spvShader,
                   const vk::raii::Device &device) {

  std::ifstream file(spvShader,
                     std::ios::ate | std::ios::binary); // from end of file
  if (!file.is_open()) {
    throw std::runtime_error("Error:failed to open file !");
  }

  // read file to buffer
  std::vector<char> buffer(file.tellg());
  file.seekg(0, std::ios::beg);
  file.read(buffer.data(), static_cast<std::streamsize>(buffer.size()));
  file.close();

  vk::ShaderModuleCreateInfo createInfo{
      .codeSize = buffer.size() * sizeof(char),
      .pCode = reinterpret_cast<const uint32_t *>(buffer.data())};
  vk::raii::ShaderModule shaderModule{device, createInfo};

  return shaderModule;
}

// ------ Building pipeline ----

vk::raii::Pipeline
PipelineBuilder::build(VkContext &vkContext, Swapchain &swapchain,
                       vk::raii::PipelineLayout &pipelineLayout) {

  // load shaders
  auto shaderModule = load_shader_module(shaderPath, vkContext.device);
  vk::PipelineShaderStageCreateInfo vertShaderStageInfo{
      .stage = vk::ShaderStageFlagBits::eVertex,
      .module = shaderModule,
      .pName = "vertMain"};
  vk::PipelineShaderStageCreateInfo fragShaderStageInfo{
      .stage = vk::ShaderStageFlagBits::eFragment,
      .module = shaderModule,
      .pName = "fragMain"};
  vk::PipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo,
                                                      fragShaderStageInfo};

  // vertex stage (no attributes = no vertex buffer bound)
  bool hasVertexInput = !vertexInput.attributes.empty();

  vk::PipelineVertexInputStateCreateInfo vertexInputInfo{
      .vertexBindingDescriptionCount = hasVertexInput ? 1u : 0u,
      .pVertexBindingDescriptions =
          hasVertexInput ? &vertexInput.binding : nullptr,
      .vertexAttributeDescriptionCount =
          static_cast<uint32_t>(vertexInput.attributes.size()),
      .pVertexAttributeDescriptions = vertexInput.attributes.data()};

  // topology
  vk::PipelineInputAssemblyStateCreateInfo inputAssembly{
      .topology = vk::PrimitiveTopology::eTriangleList};

  // dynamic state
  std::vector<vk::DynamicState> dynamicStates = {vk::DynamicState::eViewport,
                                                 vk::DynamicState::eScissor};

  vk::PipelineDynamicStateCreateInfo dynamicState{
      .dynamicStateCount = static_cast<uint32_t>(dynamicStates.size()),
      .pDynamicStates = dynamicStates.data()};

  // viewport and scissor are dynamic (set each frame in the command buffer),
  // only their count is baked into the pipeline
  vk::PipelineViewportStateCreateInfo viewportState{.viewportCount = 1,
                                                    .scissorCount = 1};

  // rasterizer
  vk::PipelineRasterizationStateCreateInfo rasterizer{
      .depthClampEnable = vk::False,
      .rasterizerDiscardEnable = vk::False,
      .polygonMode = vk::PolygonMode::eFill,
      .cullMode = cullMode,
      .frontFace = vk::FrontFace::eCounterClockwise,
      .depthBiasEnable = vk::False,
      .lineWidth = 1.0f};

  // multisampling (disabled for now)
  vk::PipelineMultisampleStateCreateInfo multisampling{
      .rasterizationSamples = vkContext.msaaSamples,
      //.sampleShadingEnable = vk::False
  };

  // color blending
  vk::PipelineColorBlendAttachmentState colorBlendAttachment{
      .blendEnable = blendEnable,
      .srcColorBlendFactor = vk::BlendFactor::eSrcAlpha,
      .dstColorBlendFactor = vk::BlendFactor::eOneMinusSrcAlpha,
      .colorBlendOp = vk::BlendOp::eAdd,
      .srcAlphaBlendFactor = vk::BlendFactor::eOne,
      .dstAlphaBlendFactor = vk::BlendFactor::eZero,
      .alphaBlendOp = vk::BlendOp::eAdd,
      .colorWriteMask =
          vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG |
          vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA};

  vk::PipelineColorBlendStateCreateInfo colorBlending{
      .logicOpEnable = vk::False,
      .logicOp = vk::LogicOp::eCopy,
      .attachmentCount = 1,
      .pAttachments = &colorBlendAttachment};

  // pipeline layout
  // specify uniform variables and push constants
  // TODO : add setter for pipeline layout

  // depth buffering
  vk::PipelineDepthStencilStateCreateInfo depthStencil{
      .depthTestEnable = depthTestEnable,
      .depthWriteEnable = depthWriteEnable,
      .depthCompareOp = vk::CompareOp::eLess,
      .depthBoundsTestEnable = vk::False,
      .stencilTestEnable = vk::False};

  // dynamic rendering: attachment formats must match the ones used in
  // beginRendering
  vk::Format colorFormat = swapchain.get_surface_format().format;

  vk::StructureChain<vk::GraphicsPipelineCreateInfo,
                     vk::PipelineRenderingCreateInfo>
      pipelineCreateInfoChain = {
          {.stageCount = 2,
           .pStages = shaderStages,
           .pVertexInputState = &vertexInputInfo,
           .pInputAssemblyState = &inputAssembly,
           .pViewportState = &viewportState,
           .pRasterizationState = &rasterizer,
           .pMultisampleState = &multisampling,
           .pDepthStencilState = &depthStencil,
           .pColorBlendState = &colorBlending,
           .pDynamicState = &dynamicState,
           .layout = pipelineLayout,
           .renderPass = nullptr},
          {.colorAttachmentCount = 1,
           .pColorAttachmentFormats = &colorFormat,
           .depthAttachmentFormat = (vk::Format)VK_FORMAT_D32_SFLOAT}};

  vk::raii::Pipeline graphicsPipeline = vk::raii::Pipeline(
      vkContext.device, nullptr,
      pipelineCreateInfoChain.get<vk::GraphicsPipelineCreateInfo>());

  return std::move(graphicsPipeline);
}

// ---- setters
// PipelineBuilder &PipelineBuilder::setTopology(vk::PrimitiveTopology
// topology)
// {
//  this->topology = topology;
//
//  return *this;
//}

PipelineBuilder &PipelineBuilder::set_shader(std::string shaderPath) {
  this->shaderPath = shaderPath;

  return *this;
}

PipelineBuilder &
PipelineBuilder::set_vertex_input(VertexInputDescription vertexInput) {
  this->vertexInput = std::move(vertexInput);
  return *this;
}

PipelineBuilder &PipelineBuilder::set_depth_test(bool testEnable,
                                                 bool writeEnable) {
  depthTestEnable = testEnable;
  depthWriteEnable = writeEnable;
  return *this;
}

PipelineBuilder &PipelineBuilder::set_cull_mode(vk::CullModeFlags cullMode) {
  this->cullMode = cullMode;
  return *this;
}

PipelineBuilder &PipelineBuilder::set_blending(bool enable) {
  blendEnable = enable;
  return *this;
}
