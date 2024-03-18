# Add executable for COMP203
add_executable(main main.cpp main.h)

# Link the LZ77 library to COMP203
target_link_libraries(main LZ77)
target_link_libraries(main Huffman)
include_directories(${CMAKE_SOURCE_DIR}/Huffman)
include_directories(${CMAKE_SOURCE_DIR}/LZ77)