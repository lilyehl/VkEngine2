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
  auto shaderModule = load_shader_module("shaders/slang.spv", vkContext.device);
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

  // vertex stage
  auto bindingDescription = Vertex::getBindingDescription();
  auto attributeDescriptions = Vertex::getAttributeDescriptions();

  vk::PipelineVertexInputStateCreateInfo vertexInputInfo{
      .vertexBindingDescriptionCount = 1,
      .pVertexBindingDescriptions = &bindingDescription,
      .vertexAttributeDescriptionCount =
          static_cast<uint32_t>(attributeDescriptions.size()),
      .pVertexAttributeDescriptions = attributeDescriptions.data()};

  // topology
  vk::PipelineInputAssemblyStateCreateInfo inputAssembly{
      .topology = vk::PrimitiveTopology::eTriangleList};

  // dynamic state
  std::vector<vk::DynamicState> dynamicStates = {vk::DynamicState::eViewport,
                                                 vk::DynamicState::eScissor};

  vk::PipelineDynamicStateCreateInfo dynamicState{
      .dynamicStateCount = static_cast<uint32_t>(dynamicStates.size()),
      .pDynamicStates = dynamicStates.data()};

  // viewport and scissors
  const vk::Extent2D &swapChainExtent = swapchain.get_extent();

  vk::Viewport viewport{0.0f,
                        0.0f,
                        static_cast<float>(swapChainExtent.width),
                        static_cast<float>(swapChainExtent.height),
                        0.0f,
                        1.0f};
  vk::Rect2D scissor{vk::Offset2D{0, 0},
                     swapChainExtent}; // same size as viewport

  vk::PipelineViewportStateCreateInfo viewportState{.viewportCount = 1,
                                                    .scissorCount = 1};

  // rasterizer
  vk::PipelineRasterizationStateCreateInfo rasterizer{
      .depthClampEnable = vk::False,
      .rasterizerDiscardEnable = vk::False,
      .polygonMode = vk::PolygonMode::eFill,
      .cullMode = vk::CullModeFlagBits::eBack,
      .frontFace = vk::FrontFace::eCounterClockwise,
      .depthBiasEnable = vk::False,
      .lineWidth = 1.0f};

  // multisampling (disabled for now)
  vk::PipelineMultisampleStateCreateInfo multisampling{
      .rasterizationSamples = vk::SampleCountFlagBits::e1,
      .sampleShadingEnable = vk::False};

  // color blending
  vk::PipelineColorBlendAttachmentState colorBlendAttachment{
      .blendEnable = vk::True,
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

  // dynamic rendering
  vk::Format colorFormat = swapchain.get_surface_format().format;
  vk::PipelineRenderingCreateInfo pipelineRenderingCreateInfo{
      .colorAttachmentCount = 1, .pColorAttachmentFormats = &colorFormat};

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
           .pColorBlendState = &colorBlending,
           .pDynamicState = &dynamicState,
           .layout = pipelineLayout,
           .renderPass = nullptr},
          {.colorAttachmentCount = 1, .pColorAttachmentFormats = &colorFormat}};

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
