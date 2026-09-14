# Vulkan wrapper

dependencies :
build : cmake/ninja/clang
vulkan : vulkan-headers/vulkan-validation-layers

---

Build project :

```
chmod +x shaders/shaders.sh
./shaders/shaders.sh
git submodule update --init
cmake -S . -B build -G Ninja
ninja -C build
```
