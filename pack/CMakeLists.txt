add_library(libpack STATIC pack.cpp unpack.cpp pack_utils.cpp)
target_include_directories(libpack PUBLIC .)