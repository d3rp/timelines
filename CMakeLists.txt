cmake_minimum_required(VERSION 3.11)

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

# json lib
include(FetchContent)

FetchContent_Declare(json
  GIT_REPOSITORY https://github.com/nlohmann/json.git
  GIT_TAG v3.7.3)

FetchContent_GetProperties(json)
if(NOT json_POPULATED)
  FetchContent_Populate(json)
  add_subdirectory(${json_SOURCE_DIR} ${json_BINARY_DIR} EXCLUDE_FROM_ALL)
endif()

target_link_libraries(
    ${PROJECT_NAME}
    CONAN_PKG::sdl
    CONAN_PKG::sdl_ttf
    CONAN_PKG::cpr
    nlohmann_json::nlohmann_json
)
#target_link_libraries(foo PRIVATE nlohmann_json::nlohmann_json)
