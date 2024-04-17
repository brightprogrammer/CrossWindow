find_package(Vulkan REQUIRED)
find_package(PkgConfig)

add_subdirectory(Common)

pkg_check_modules(XCB xcb xcb-keysyms xcb-icccm)
if(${XCB_FOUND})
  add_subdirectory(Platform/XCB)
else()
  message(FATAL_ERROR "Host platform is not supported/recognized. Please check supported platforms or install required libraries.")
endif()
  
# create crosswindow as interface library by selecting Platform
# dependent library
add_library(crosswindow INTERFACE)
target_link_libraries(crosswindow INTERFACE ${CrossWindow_LIBRARIES}) 
