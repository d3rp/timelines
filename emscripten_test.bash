#!/bin/bash
# check if requirements to use emscripten are met
# https://emscripten.org/docs/building_from_source/toolchain_what_is_needed.html?highlight=cmake

# Check for Python
python --version

# Check for node.js on Linux
nodejs --version

# Check for node.js on Windows
node --version

# Check for node.js on macOS
node -v

# Check for git
git --version

# Check for Java
java -version

# Check for gcc / g++
gcc --version
g++

# Check for cmake
cmake
