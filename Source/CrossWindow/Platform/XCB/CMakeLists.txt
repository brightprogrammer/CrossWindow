# Vulkan is already found in Source/CMakeLists.txt

file(GLOB_RECURSE CROSSWINDOW_XCB_SRC_FILES ${CMAKE_CURRENT_SOURCE_DIR} *.c)

add_library(crosswindow_xcb SHARED ${CROSSWINDOW_XCB_SRC_FILES})
target_include_directories(crosswindow_xcb PUBLIC ${XCB_INCLIDE_DIRS} ${Vulkan_INCLUDE_DIRS})
target_link_libraries(crosswindow_xcb crosswindow_common ${XCB_LIBRARIES} ${Vulkan_LIBRARIES})

# install librarry
install(TARGETS crosswindow_xcb LIBRARY DESTINATION ${INSTALL_LIB_DIR})
crosswindow_add_library_name("crosswindow_xcb")
