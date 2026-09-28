# network_ee_sample CMakeLists.txt

add_executable(network_ee_sample.elf network_ee_sample.c)

target_include_directories(network_ee_sample.elf PRIVATE
    ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(network_ee_sample.elf
    ${CMAKE_BINARY_DIR}/${EE_LIB_COMBINED}
    -lpatches
    -ldebug
)

target_compile_options(network_ee_sample.elf PRIVATE -O0 -g)

set_target_properties(network_ee_sample.elf PROPERTIES
    LINK_FLAGS "-L${CMAKE_BINARY_DIR}"
)

# Add dependency on the main library
add_dependencies(network_ee_sample.elf ps2_drivers)
