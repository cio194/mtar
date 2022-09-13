add_library(libcompress STATIC compress.cpp huffman_tree.cpp decompress.cpp
        compress_utils.cpp)
target_include_directories(libcompress PUBLIC .)