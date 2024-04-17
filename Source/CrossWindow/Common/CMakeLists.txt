set(CROSSWINDOW_COMMON_SRC_FILES Event.c) 

add_library(crosswindow_common SHARED ${CROSSWINDOW_COMMON_SRC_FILES})

install(TARGETS crosswindow_common LIBRARY DESTINATION lib)
crosswindow_add_library_name("crosswindow_common")
