#include "terrain.hpp"

float Perlin::fade(float t) { return t * t * t * (t * (t * 6 - 15) + 10); }

uint Perlin::pcg_hash(uint input) {
  // Basically a good hash function that takes in a uint and spits out another
  // uint
  // According to this post :
  // https://www.reedbeta.com/blog/hash-functions-for-gpu-rendering/
  uint state = input * 747796405u + 2891336453u;
  uint word = ((state >> ((state >> 28u) + 4u)) ^ state) * 277803737u;
  return (word >> 22u) ^ word;
}

float Perlin::noise(float x, float y) {

  // a random gradient vector is assigned to each point on the lattice
  // g = < [-1, 1], [-1, 1] >
  // using the pcg hash function ?

  auto grad = [x, y](uint cx, uint cy) {
    // (x,y) is a point within a square of the lattice
    // for each corner of the square, eval the dot prod between
    // grad vect and the offset vector from the corner to (x,y)

    uint h = pcg_hash(cx) + pcg_hash(cy);

    // map hash to an angle in [0, 2pi]
    float a = (float)h * 2.f * M_PI;

    glm::vec2 g = glm::vec2(sin(a), cos(a));           // gradient vector
    glm::vec2 d = glm::vec2(x, y) - glm::vec2(cx, cy); // offset vector

    return glm::dot(g, d);
  };

  // bottom-left corner
  int X = std::floor(x);
  int Y = std::floor(y);

  // fade curves w/ 5th degree interpolation
  float u = fade(x - X);
  float v = fade(y - Y);

  float noise = std::lerp(std::lerp(grad(X, Y), grad(X + 1, Y), u),
                          std::lerp(grad(X, Y + 1), grad(X + 1, Y + 1), u), v);

  return noise;
}

void Terrain::generate() {

  for (int x = 0; x < WIDTH; ++x) {
    for (int z = 0; z < WIDTH; ++z) {

      float y = Perlin::noise((float)x / SCALE, (float)z / SCALE);
      Voxel v = Voxel({x, y, z});
      uint16_t baseIdx = static_cast<uint16_t>(voxels.vertices.size());

      voxels.vertices.insert(voxels.vertices.end(), v.vertices.begin(),
                             v.vertices.end());

      for (auto idx : v.indices)
        voxels.indices.push_back(baseIdx + idx);
    }
  }
}
