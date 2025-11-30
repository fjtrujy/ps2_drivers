# showiopinfo_sample CMakeLists.txt

add_executable(showiopinfo_sample.elf showiopinfo_sample.c)

target_include_directories(showiopinfo_sample.elf PRIVATE
    ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(showiopinfo_sample.elf
    -lps2_drivers
    -lpatches
)

target_compile_options(showiopinfo_sample.elf PRIVATE -O0 -g)

set_target_properties(showiopinfo_sample.elf PROPERTIES
    LINK_FLAGS "-L${CMAKE_BINARY_DIR}"
)

# Add dependency on the main library
add_dependencies(showiopinfo_sample.elf ps2_drivers)
