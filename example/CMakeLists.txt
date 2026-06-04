add_executable(example main.cpp)

target_link_libraries(example
    PRIVATE vk_wrapper
)

#target_compile_options(example PRIVATE -fsanitize=address -fno-omit-frame-pointer)
#target_link_options(example PRIVATE -fsanitize=address)


