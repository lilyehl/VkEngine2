#pragma once

#include "utils.hpp"

// how a pipeline reads one vertex type from a vertex buffer.
// stored by value so the pipeline builder can own it until build()
struct VertexInputDescription {
  // at which rate to load data from memory throughout vertices
  vk::VertexInputBindingDescription binding;
  // how to extract each attribute from a chunk of vertex data
  std::vector<vk::VertexInputAttributeDescription> attributes;
};

using Vert = glm::vec3; // in case I want to make it a struct later
using Vertices = std::vector<glm::vec3>;

struct Vertex {
  // TODO : clean this up and change it to Vertex3D

  static VertexInputDescription description() {
    return {.binding = {.binding = 0,
                        .stride = sizeof(Vert),
                        .inputRate = vk::VertexInputRate::eVertex},
            .attributes = {
                // position
                {.location = 0,
                 .binding = 0,
                 .format = vk::Format::eR32G32B32Sfloat,
                 .offset = 0},
            }};
  }
};

struct Vertex2D {
  glm::vec2 pos;
  glm::vec3 color;
  glm::vec2 texCoord;

  static VertexInputDescription description() {
    return {.binding = {.binding = 0,
                        .stride = sizeof(Vertex2D),
                        .inputRate = vk::VertexInputRate::eVertex},
            .attributes = {
                {.location = 0,
                 .binding = 0,
                 .format = vk::Format::eR32G32Sfloat,
                 .offset = offsetof(Vertex2D, pos)},
                {.location = 1,
                 .binding = 0,
                 .format = vk::Format::eR32G32B32Sfloat,
                 .offset = offsetof(Vertex2D, color)},
                {.location = 2,
                 .binding = 0,
                 .format = vk::Format::eR32G32Sfloat,
                 .offset = offsetof(Vertex2D, texCoord)},
            }};
  }
};
