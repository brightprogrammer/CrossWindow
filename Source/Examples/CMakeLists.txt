set(MAIN_SRCS Main.c)

add_executable(main ${MAIN_SRCS})
target_link_libraries(main crosswindow)
