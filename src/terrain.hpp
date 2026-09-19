#pragma once

#include "vk/vk_mesh.hpp"
#include "window.hpp"
#include <glm/glm.hpp>
#include <vector>

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

class Terrain {
public:
  MeshData voxels;
  int WIDTH, HEIGHT;

  Terrain(int width = 15, int height = 15) : WIDTH(width), HEIGHT(height) {};

  void generate() {
    // perlin noise

    // cubes to coords

    // temp stupid setup of just 1 cube
    for (int x = 0; x < WIDTH; ++x) {
      for (int z = 0; z < WIDTH; ++z) {

        Voxel v = Voxel({x, 0, z});
        uint16_t baseIdx = static_cast<uint16_t>(voxels.vertices.size());

        voxels.vertices.insert(voxels.vertices.end(), v.vertices.begin(),
                               v.vertices.end());

        for (auto idx : v.indices)
          voxels.indices.push_back(baseIdx + idx);
      }
    }
  }

private:
};
