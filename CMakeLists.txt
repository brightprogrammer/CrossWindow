cmake_minimum_required(VERSION 3.5)

# project settings
project(V0 VERSION 0 LANGUAGES C)

# set directories where binary files will be stored
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib)

# export the compile_commands.json file
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# set compiler flags to generate extra warnings and treat them as error
# set(CMAKE_C_FLAGS "${CMAKE_C_CLAGS} -ggdb -Wall -Wextra -Werror -fsanitize=address")
set(CMAKE_C_FLAGS "${CMAKE_C_CLAGS} -ggdb -Wall -Wextra -Werror")

include_directories(Include)
add_subdirectory(Source)

