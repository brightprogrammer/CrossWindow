find_package(PkgConfig)
find_package(Vulkan REQUIRED)

add_subdirectory(Common)

# create crosswindow as interface library by selecting Platform
# dependent library
add_library(crosswindow INTERFACE)

pkg_check_modules(XCB xcb xcb-keysyms)
if(${XCB_FOUND})
  add_subdirectory(Platform/XCB)
  target_link_libraries(crosswindow INTERFACE crosswindow_xcb)
endif()

