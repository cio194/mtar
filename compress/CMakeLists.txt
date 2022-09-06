add_library(libcompress STATIC compress.cpp huffman_tree.cpp)
target_include_directories(libcompress PUBLIC .)