# hdd_sample CMakeLists.txt

add_executable(hdd_sample.elf hdd_sample.c)

target_include_directories(hdd_sample.elf PRIVATE
    ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(hdd_sample.elf
    -lps2_drivers
    -lpatches
    -ldebug
)

target_compile_options(hdd_sample.elf PRIVATE -O0 -g)

set_target_properties(hdd_sample.elf PROPERTIES
    LINK_FLAGS "-L${CMAKE_BINARY_DIR}"
)

# Add dependency on the main library
add_dependencies(hdd_sample.elf ps2_drivers)
