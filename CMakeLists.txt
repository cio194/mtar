cmake_minimum_required(VERSION 3.14)
project(mtar)

set(CMAKE_CXX_STANDARD 14)

add_subdirectory(compress)
add_subdirectory(pack)
add_subdirectory(interact)
add_subdirectory(test)