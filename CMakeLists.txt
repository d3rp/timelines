cmake_minimum_required(VERSION 3.10)

project(main)
aux_source_directory(. SRC_LIST)
add_executable(${PROJECT_NAME} ${SRC_LIST} headers.h csv/csv.h)

# C++17
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --std=c++17")
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# SDL2
#include(build/conanbuildinfo.cmake)
#conan_basic_setup(TARGETS)

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake) # Include Conan-generated file
conan_basic_setup(TARGETS) # Introduce Conan-generated targets

target_link_libraries(
    ${PROJECT_NAME}
    CONAN_PKG::sdl2
    CONAN_PKG::sdl2_ttf
    CONAN_PKG::cpr
)
