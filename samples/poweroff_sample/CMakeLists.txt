# poweroff_sample CMakeLists.txt

add_executable(poweroff_sample.elf poweroff_sample.c)

target_include_directories(poweroff_sample.elf PRIVATE
    ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(poweroff_sample.elf
    -lps2_drivers
    -lpatches
)

target_compile_options(poweroff_sample.elf PRIVATE -O0 -g)

set_target_properties(poweroff_sample.elf PROPERTIES
    LINK_FLAGS "-L${CMAKE_BINARY_DIR}"
)

# Add dependency on the main library
add_dependencies(poweroff_sample.elf ps2_drivers)
