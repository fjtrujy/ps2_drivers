# wav_sample CMakeLists.txt

add_executable(wav_sample.elf wav_sample.c)

target_include_directories(wav_sample.elf PRIVATE
    ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(wav_sample.elf
    -lps2_drivers
    -lpatches
)

target_compile_options(wav_sample.elf PRIVATE -O0 -g)

set_target_properties(wav_sample.elf PROPERTIES
    LINK_FLAGS "-L${CMAKE_BINARY_DIR}"
)

# Add dependency on the main library
add_dependencies(wav_sample.elf ps2_drivers)
