# Vulkan wrapper

### Dependencies :
build : cmake/ninja/clang \
vulkan : vulkan-headers/vulkan-validation-layers

---

### Build project :

First compile shaders with shaders/shaders.sh

```
git submodule update --init
cmake -S . -B build -G Ninja
ninja -C build
```

---

### Resources :

- Vulkan : \
[Khronos Official Vulkan Tutorial](https://docs.vulkan.org/tutorial/latest/00_Introduction.html)
- Perlin : \
[What is Perlin Noise ? - Acerola](https://www.youtube.com/watch?v=DxUY42r_6Cg) \
[Implementing Improved Perlin Noise - Ken Perlin](https://developer.nvidia.com/gpugems/gpugems/part-i-natural-effects/chapter-5-implementing-improved-perlin-noise)
