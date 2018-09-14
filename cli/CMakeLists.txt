cmake_minimum_required(VERSION 3.6)

project(lext_cli LANGUAGES C)

add_executable(cli "main.c")

target_link_libraries(cli PUBLIC lext)

target_compile_options(cli PUBLIC "-Wall")
target_compile_features(cli PUBLIC c_std_99)

set_target_properties(cli PROPERTIES
	OUTPUT_NAME "lext"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/cli"
)
