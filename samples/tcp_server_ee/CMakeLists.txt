# tcp_server_ee CMakeLists.txt — pure TCP echo server (EE-side path)

add_executable(tcp_server_ee.elf tcp_server_ee.c)

target_include_directories(tcp_server_ee.elf PRIVATE
    ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(tcp_server_ee.elf
    ${CMAKE_BINARY_DIR}/${EE_LIB_COMBINED}
    -lpatches
    -ldebug
)

target_compile_options(tcp_server_ee.elf PRIVATE -O0 -g)

set_target_properties(tcp_server_ee.elf PROPERTIES
    LINK_FLAGS "-L${CMAKE_BINARY_DIR}"
)

add_dependencies(tcp_server_ee.elf ps2_drivers)
