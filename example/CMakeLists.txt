add_executable(example main.cpp)

target_link_libraries(example
    PRIVATE vk_wrapper
)
