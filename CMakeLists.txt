# CMakeList.txt : CMake project for COMP203, include source and define
# project specific logic here.
#
# Specify the minimum version for CMake
cmake_minimum_required(VERSION 3.8)

# Project's name
project("COMP203")

# Include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

# Include subdirectories
add_subdirectory(Huffman)
add_subdirectory(LZ77)
add_subdirectory(main)
include_directories("/home/git/xxHash")
link_directories("/home/git/xxHash")

set(CMAKE_CXX_STANDARD 17)
set_property(GLOBAL PROPERTY CXX_STANDARD 20)

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(GLOBAL PROPERTY CXX_STANDARD 20)
endif()


# TODO: Add tests and install targets if needed.