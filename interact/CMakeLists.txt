add_library(libinteract STATIC interact.cpp)
target_include_directories(libinteract PUBLIC .)
target_link_libraries(libinteract PUBLIC libpack)
target_link_libraries(libinteract PUBLIC libcompress)