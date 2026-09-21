add_library(vk_wrapper STATIC)
add_subdirectory(vk)

add_executable(app main.cpp)
target_sources(app
    PRIVATE
    window.cpp
    camera.cpp
    terrain.cpp
)

target_link_libraries(app
    PRIVATE
    vk_wrapper
)
