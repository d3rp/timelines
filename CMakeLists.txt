cmake_minimum_required(VERSION 3.1)

project(main)
aux_source_directory(. SRC_LIST)
add_executable(${PROJECT_NAME} ${SRC_LIST} dot.h entities.h)

# C++17
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")

# SDL2
include(build/conanbuildinfo.cmake)
conan_basic_setup(TARGETS)
target_link_libraries(${PROJECT_NAME} CONAN_PKG::sdl2 CONAN_PKG::sdl2_ttf)
