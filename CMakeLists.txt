cmake_minimum_required(VERSION 3.10)
cmake_policy(SET CMP0079 NEW)

project(vkEngine)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_CXX_COMPILER clang++)

find_package(Vulkan REQUIRED)

add_subdirectory(vulkan)
add_subdirectory(example)
#add_subdirectory(dep)

