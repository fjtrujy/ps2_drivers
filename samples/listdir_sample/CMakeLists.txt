# listdir_sample CMakeLists.txt

add_executable(listdir_sample.elf listdir_sample.c)

target_include_directories(listdir_sample.elf PRIVATE
    ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(listdir_sample.elf
    -lps2_drivers
    -lpatches
    -ldebug
)

target_compile_options(listdir_sample.elf PRIVATE -O0 -g)

set_target_properties(listdir_sample.elf PROPERTIES
    LINK_FLAGS "-L${CMAKE_BINARY_DIR}"
)

# Add dependency on the main library
add_dependencies(listdir_sample.elf ps2_drivers)
