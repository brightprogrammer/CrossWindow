cmake_minimum_required(VERSION 3.5)

# project settings
project(CrossWindow VERSION 0 LANGUAGES C)

# set directories where binary files will be stored
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib)

# export the compile_commands.json file
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# set compiler flags to generate extra warnings and treat them as error
# set(CMAKE_C_FLAGS "${CMAKE_C_CLAGS} -ggdb -Wall -Wextra -Werror -fsanitize=address")
set(CMAKE_C_FLAGS "${CMAKE_C_CLAGS} -ggdb -Wall -Wextra -Werror")

# for getting install dirs
include (GNUInstallDirs)
set(CrossWindow_INCLUDE_DIR "${CMAKE_INSTALL_FULL_INCLUDEDIR}")
set(CrossWindow_LIB_DIR "${CMAKE_INSTALL_FULL_LIBDIR}")

# install header files in platform specific include directories
install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/Include/" DESTINATION ${CrossWindow_INCLUDE_DIR})

# include the Include directory project-wide
include_directories(Include)

# function to add library name to CrossWindow_LIBRARIES variable
set_property(GLOBAL PROPERTY CrossWindow_LIBRARIES)
function(crosswindow_add_library_name)
    get_property(tmp GLOBAL PROPERTY CrossWindow_LIBRARIES)
    foreach(arg ${ARGV})
      if(NOT tmp)
          set(tmp "-l${arg}")
        else()
          set(tmp "${tmp} -l${arg}")
        endif()
    endforeach()
    set_property(GLOBAL PROPERTY CrossWindow_LIBRARIES "${tmp}")
endfunction(crosswindow_add_library_name)

# the source subdirectoy will define some variables necessary for
# creating pkg-config install files 
add_subdirectory(Source)

# get names of all libraries in crosswindow
get_property(CrossWindow_LIBRARY_NAMES GLOBAL PROPERTY CrossWindow_LIBRARIES)

# Configure and install pkg-config file
configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/CMake/crosswindow.pc.in"
  "${CMAKE_CURRENT_BINARY_DIR}/crosswindow.pc"
  @ONLY
)
install(
  FILES "${CMAKE_CURRENT_BINARY_DIR}/crosswindow.pc"
  DESTINATION "lib/pkgconfig"
)

set(CPACK_OUTPUT_FILE_PREFIX "${CMAKE_CURRENT_BINARY_DIR}/packages")
include(CPack)
