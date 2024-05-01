# Add LZ77 as a library
add_library(LZ77 LZ77.cpp LZ77.h)
target_link_libraries(LZ77 xxhash)