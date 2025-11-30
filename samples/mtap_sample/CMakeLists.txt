# mtap_sample CMakeLists.txt

add_executable(mtap_sample.elf mtap_sample.c)

target_include_directories(mtap_sample.elf PRIVATE
    ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(mtap_sample.elf
    -lps2_drivers
    -lpatches
)

target_compile_options(mtap_sample.elf PRIVATE -O0 -g)

set_target_properties(mtap_sample.elf PROPERTIES
    LINK_FLAGS "-L${CMAKE_BINARY_DIR}"
)

# Add dependency on the main library
add_dependencies(mtap_sample.elf ps2_drivers)
