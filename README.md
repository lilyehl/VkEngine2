# Vulkan wrapper

dependencies :
build : cmake/ninja/clang
vulkan : vulkan-headers/vulkan-validation-layers

---

```
git submodule update --init
cmake -S . -B build -G Ninja
ninja -C build
```
