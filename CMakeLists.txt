cmake_minimum_required(VERSION 3.6)

project(lext LANGUAGES C)

add_library(lext STATIC 
	"src/lext.c"
	"src/cursor.c"
	"src/token.c"
	"src/template.c"
)

target_include_directories(lext PUBLIC "include")

target_compile_options(lext PRIVATE "-Wall")
target_compile_features(lext PRIVATE c_std_99)

set_target_properties(lext PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
)

option(LEXT_BUILD_EXAMPLES "Build example programs" ON)

if(${LEXT_BUILD_EXAMPLES})
    add_subdirectory("example")
endif()
