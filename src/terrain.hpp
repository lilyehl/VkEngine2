#pragma once

#include "vk/utils.hpp"
#include "vk/vk_mesh.hpp"
#include "window.hpp"

// stick this into a vertex shader ? only send in coordinates of the voxels ?
struct Voxel {
  Vertices vertices = {{-0.5f, -0.5f, 0.5f},  {0.5f, -0.5f, 0.5f},
                       {-0.5f, 0.5f, 0.5f},   {0.5f, 0.5f, 0.5f},
                       {-0.5f, -0.5f, -0.5f}, {0.5f, -0.5f, -0.5f},
                       {-0.5f, 0.5f, -0.5f},  {0.5f, 0.5f, -0.5f}};

  std::vector<uint16_t> indices = {
      // Top
      2, 7, 6, 2, 3, 7,
      // Bottom
      0, 4, 5, 0, 5, 1,
      // Left
      0, 2, 6, 0, 6, 4,
      // Right
      1, 7, 3, 1, 5, 7,
      // Front
      0, 3, 2, 0, 1, 3,
      // Back
      4, 6, 7, 4, 7, 5};

  Voxel() = default;

  explicit Voxel(glm::vec3 offset) {
    for (auto &v : vertices)
      v += offset;
  }
};

class Perlin {
public:
  // 5th order interpolation solves discontinuous continuity
  static float fade(float t);

  static uint pcg_hash(uint input);

  static float noise(float x, float y);
};

class Terrain {
public:
  MeshData voxels;
  int WIDTH, HEIGHT;
  float SCALE;

  Terrain(int width = 100, int height = 0, int scale = 5)
      : WIDTH(width), HEIGHT(height), SCALE(scale) {};

  void generate();
};
