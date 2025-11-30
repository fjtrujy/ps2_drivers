# alldrivers_sample CMakeLists.txt

add_executable(alldrivers_sample.elf alldrivers_sample.c)

target_include_directories(alldrivers_sample.elf PRIVATE
    ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(alldrivers_sample.elf
    -lps2_drivers
    -lpatches
)

target_compile_options(alldrivers_sample.elf PRIVATE -O0 -g)

set_target_properties(alldrivers_sample.elf PROPERTIES
    LINK_FLAGS "-L${CMAKE_BINARY_DIR}"
)

# Add dependency on the main library
add_dependencies(alldrivers_sample.elf ps2_drivers)
