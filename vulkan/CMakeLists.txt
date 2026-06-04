# set up Vulkan C++ module (enabled when ENABLE_CPP20_MODULE=ON)
#add_library(VulkanCppModule)
#add_library(Vulkan::cppm ALIAS VulkanCppModule)
#

#
#target_include_directories(VulkanCppModule PRIVATE "${Vulkan_INCLUDE_DIR}")
#target_link_libraries(VulkanCppModule PUBLIC Vulkan::Vulkan)
#
#set_target_properties(VulkanCppModule PROPERTIES CXX_STANDARD 20)
#
#target_sources(VulkanCppModule
#        PUBLIC
#        FILE_SET cxx_modules TYPE CXX_MODULES
#        BASE_DIRS "${Vulkan_INCLUDE_DIR}"
#        FILES "${Vulkan_INCLUDE_DIR}/vulkan/vulkan.cppm"
#)

# set up my wrapper
add_library(vk_wrapper STATIC)


target_compile_definitions(vk_wrapper PUBLIC
        VULKAN_HPP_DISPATCH_LOADER_DYNAMIC=1
        VULKAN_HPP_NO_STRUCT_CONSTRUCTORS=1
)

target_precompile_headers( vk_wrapper
  PRIVATE
  utils.hpp
)

target_sources(vk_wrapper
    PRIVATE
        vk_engine.cpp
        vk_context.cpp
        vk_swapchain.cpp
        vk_pipeline.cpp
)

target_include_directories(vk_wrapper PUBLIC
     ${CMAKE_CURRENT_SOURCE_DIR}
)
target_link_libraries(vk_wrapper PUBLIC
    #Vulkan::cppm
    ${Vulkan_LIBRARIES}
    glfw
)


