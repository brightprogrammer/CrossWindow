add_subdirectory(Shaders)

add_executable(triangle Triangle.c)

find_package(Vulkan REQUIRED)

get_property(CrossWindow_LIBRARY_NAMES GLOBAL PROPERTY CrossWindow_LIBRARIES)
target_link_libraries(triangle ${CrossWindow_LIBRARY_NAMES} m ${Vulkan_LIBRARIES}) 
