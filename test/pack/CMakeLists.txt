add_executable(pack_test pack_test.cpp)
set_property(TARGET pack_test PROPERTY RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(pack_test PUBLIC libpack)