# filesystem_sample CMakeLists.txt

add_executable(filesystem_sample.elf filesystem_sample.c)

target_include_directories(filesystem_sample.elf PRIVATE
    ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(filesystem_sample.elf
    -lps2_drivers
    -lpatches
)

target_compile_options(filesystem_sample.elf PRIVATE
    -Os -g
    -fdata-sections -ffunction-sections -flto
)

set_target_properties(filesystem_sample.elf PROPERTIES
    LINK_FLAGS "-L${CMAKE_BINARY_DIR} -fdata-sections -ffunction-sections -flto -Wl,--gc-sections"
)

# Add dependency on the main library
add_dependencies(filesystem_sample.elf ps2_drivers)
