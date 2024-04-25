# Add executable for COMP203
add_executable(main main.cpp main.h)

# Find the Google Benchmark package
find_package(benchmark REQUIRED)

# Link the Google Benchmark library to COMP203
target_link_libraries(main benchmark::benchmark)

# Link the LZ77 library to COMP203
target_link_libraries(main LZ77)
target_link_libraries(main Huffman)

include_directories(${CMAKE_SOURCE_DIR}/Huffman)
include_directories(${CMAKE_SOURCE_DIR}/LZ77)