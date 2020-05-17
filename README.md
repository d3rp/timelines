Timelines
=========

WIP, Linux and Mac

Requirements
------------

* conan
* CMake
* Compiler that supports C++14 (recent version of g++ or clang++)
* SDL2
* ninja (or make, just drop the `-GNinja` flag and build with `make`)

One way of building with C++14, SDL2, CMake and ninja
-----------------------------------------------------

    # Conan installation and configuration
    pip install conan
    conan remote update bincrafters https://api.bintray.com/conan/bincrafters/public-conan
    
    # installing dependencies (See MacOs section for troubleshooting)
    mkdir -p build
    cd build
    conan install ..
    
    # CMakeLists.txt can now be used as a project in CLion or build manually
    cmake -GNinja ..
    ninja
    cd ..
    
MacOs
-----

Might require forcing to build from source

    conan install .. --build bzip2 --build libpng --build freetype --build sdl2_ttf

Running
-------

    build/main

Cleaning up the binary file and build directory
-----------------------------------------------

    rm -rf build/ main

License
-----------------------------------------------

Licenses are grouped under directories. The child level LICENSE file overrides the parent level equivalent, for example, LICENSE at the root of this repo applies to the code inside the root directory of the repo and other child directories where an explicit LICENSE file is not available - when the LICENSE file under vera-fonts directory applies to the files inside said directory like the dejavusans ttffile.

