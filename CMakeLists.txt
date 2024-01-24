cmake_minimum_required(VERSION 3.21)

project(main)
aux_source_directory(timelines SRC_LIST)
aux_source_directory(timelines/details DETAILS_SRC_LIST)

add_executable(${PROJECT_NAME} ${SRC_LIST} ${DETAILS_SRC_LIST} timelines/headers.hpp timelines/details/csv/csv.h timelines/graphics.hpp)
set_target_properties(${PROJECT_NAME} PROPERTIES LINKER_LANGUAGE CXX)

# C++17
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --std=c++17")
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


# json lib
include(FetchContent)

FetchContent_Declare(json
        GIT_REPOSITORY https://github.com/nlohmann/json.git
        GIT_TAG v3.7.3)

FetchContent_GetProperties(json)
if (NOT json_POPULATED)
    FetchContent_Populate(json)
    add_subdirectory(${json_SOURCE_DIR} ${json_BINARY_DIR} EXCLUDE_FROM_ALL)
endif ()


find_package(BZip2 REQUIRED)
#target_link_libraries(main PRIVATE BZip2::BZip2)

find_package(ZLIB REQUIRED)
#target_link_libraries(main PRIVATE ZLIB::ZLIB)

# this is heuristically generated, and may not be correct
find_package(cpr CONFIG REQUIRED)
#target_link_libraries(main PRIVATE cpr::cpr)

find_package(SDL2 CONFIG REQUIRED)
#target_link_libraries(main
#        PRIVATE
#        $<TARGET_NAME_IF_EXISTS:SDL2::SDL2main>
#        $<IF:$<TARGET_EXISTS:SDL2::SDL2>,SDL2::SDL2,SDL2::SDL2-static>
#)

find_package(SDL2_ttf CONFIG REQUIRED)
#target_link_libraries(main PRIVATE $<IF:$<TARGET_EXISTS:SDL2_ttf::SDL2_ttf>,SDL2_ttf::SDL2_ttf,SDL2_ttf::SDL2_ttf-static>)

find_package(nlohmann_json CONFIG REQUIRED)

#set(nlohmann-json_IMPLICIT_CONVERSIONS OFF)

set(third_party_libs
        BZip2::BZip2
        ZLIB::ZLIB
        cpr::cpr
        $<TARGET_NAME_IF_EXISTS:SDL2::SDL2main>
        $<IF:$<TARGET_EXISTS:SDL2::SDL2>,SDL2::SDL2,SDL2::SDL2-static>
        $<IF:$<TARGET_EXISTS:SDL2_ttf::SDL2_ttf>,SDL2_ttf::SDL2_ttf,SDL2_ttf::SDL2_ttf-static>
        nlohmann_json::nlohmann_json
)
target_link_libraries(
        ${PROJECT_NAME}
        PRIVATE
        ${third_party_libs}
)
