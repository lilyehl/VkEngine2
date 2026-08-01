add_subdirectory(glm)
add_subdirectory(glfw)

target_link_libraries(vk_wrapper PRIVATE glfw)
#target_link_libraries(vk_wrapper PRIVATE glm)
target_include_directories(vk_wrapper PUBLIC glm)
