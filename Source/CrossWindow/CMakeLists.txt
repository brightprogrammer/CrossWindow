find_package(Vulkan REQUIRED)

add_subdirectory(Common)
add_subdirectory(Platform/XCB)

# create crosswindow as interface library by selecting Platform
# dependent library
add_library(crosswindow INTERFACE)
target_link_libraries(crosswindow INTERFACE crosswindow_xcb)
