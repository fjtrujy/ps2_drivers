# udp_echo_server_ee CMakeLists.txt — UDP echo server (EE-side path)

add_executable(udp_echo_server_ee.elf udp_echo_server_ee.c)

target_include_directories(udp_echo_server_ee.elf PRIVATE
    ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(udp_echo_server_ee.elf
    ${CMAKE_BINARY_DIR}/${EE_LIB_COMBINED}
    -lpatches
    -ldebug
)

target_compile_options(udp_echo_server_ee.elf PRIVATE -O0 -g)

set_target_properties(udp_echo_server_ee.elf PROPERTIES
    LINK_FLAGS "-L${CMAKE_BINARY_DIR}"
)

add_dependencies(udp_echo_server_ee.elf ps2_drivers)
