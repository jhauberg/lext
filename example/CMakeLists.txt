cmake_minimum_required(VERSION 3.6)

project(lext_examples LANGUAGES C)

function(add_example name)
    add_executable(${name} ${name}.c)
    
    target_link_libraries(${name} PUBLIC lext)

    target_compile_options(${name} PUBLIC "-Wall")
    target_compile_features(${name} PUBLIC c_std_99)

    set_target_properties(${name} PROPERTIES
	    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/example"
	)
endfunction()

add_example(simple)
