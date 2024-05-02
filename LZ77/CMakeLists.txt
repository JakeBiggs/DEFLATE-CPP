# Add LZ77 as a library
add_library(LZ77 LZ77.cpp LZ77.h)
target_link_libraries(LZ77 xxhash)
target_link_libraries(LZ77 libsais)
target_link_libraries(LZ77 sdsl divsufsort divsufsort64)
if (NOT TARGET sdsl)
    add_subdirectory(/home/git/sdsl-lite sdsl)
endif()