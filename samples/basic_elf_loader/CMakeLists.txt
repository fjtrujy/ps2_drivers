# basic_elf_loader CMakeLists.txt

add_executable(basic_elf_loader.elf basic_elf_loader.c)

target_include_directories(basic_elf_loader.elf PRIVATE
    ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(basic_elf_loader.elf
    -lps2_drivers
    -lpatches
    # Note: -lloadfile removed as it's not available in this toolchain build
)

target_compile_options(basic_elf_loader.elf PRIVATE -O0 -g)

set_target_properties(basic_elf_loader.elf PROPERTIES
    LINK_FLAGS "-L${CMAKE_BINARY_DIR}"
)

# Add dependency on the main library
add_dependencies(basic_elf_loader.elf ps2_drivers)
