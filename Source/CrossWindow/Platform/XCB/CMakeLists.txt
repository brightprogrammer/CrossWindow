# Vulkan is already found in Source/CMakeLists.txt

file(GLOB_RECURSE CROSSWINDOW_XCB_SRC_FILES ${CMAKE_CURRENT_SOURCE_DIR} *.c)

add_library(crosswindow_xcb ${CROSSWINDOW_XCB_SRC_FILES})
target_include_directories(crosswindow_xcb PRIVATE ${XCB_INCLIDE_DIRS} ${Vulkan_INCLUDE_DIRS})
target_link_libraries(crosswindow_xcb crosswindow_common ${XCB_LIBRARIES} ${Vulkan_LIBRARIES})
