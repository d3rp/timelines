SDL2, C++11 and CMake
=====================

Requirements
------------

* CMake
* Compiler that supports C++ (recent version of g++ or clang++)
* SDL2
* ninja (or make, just drop the `-GNinja` flag and build with `make`)

One way of building with C++11, SDL2, CMake and ninja
-----------------------------------------------------

    mkdir -p build
    cd build
    cmake -GNinja ..
    ninja
    cd ..
    
Running
-------

    build/main

Cleaning up the binary file and build directory
-----------------------------------------------

    rm -rf build/ main

License
-----------------------------------------------

Licenses are grouped under directories. The child level LICENSE file overrides the parent level equivalent, for example, LICENSE at the root of this repo applies to the code inside the root directory of the repo and other child directories where an explicit LICENSE file is not available - when the LICENSE file under vera-fonts directory applies to the files inside said directory like the dejavusans ttffile.

