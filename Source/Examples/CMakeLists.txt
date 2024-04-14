add_subdirectory(Shaders)

add_executable(triangle Triangle.c)
target_link_libraries(triangle crosswindow m)
