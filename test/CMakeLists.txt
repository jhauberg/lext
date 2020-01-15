cmake_minimum_required(VERSION 3.6)

project(lext_tests LANGUAGES C)

add_executable(test main.c)
    
target_link_libraries(test PUBLIC lext)

target_compile_options(test PUBLIC "-Wall")
target_compile_features(test PUBLIC c_std_99)

set_target_properties(test PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/test"
)
