add_subdirectory(Shaders)

add_executable(triangle Triangle.c)
target_link_libraries(main crosswindow m)
