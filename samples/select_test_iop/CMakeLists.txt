# select_test_iop CMakeLists.txt — minimal select() reproducer (IOP-side path)

add_executable(select_test_iop.elf select_test_iop.c)

target_include_directories(select_test_iop.elf PRIVATE
    ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(select_test_iop.elf
    ${CMAKE_BINARY_DIR}/${EE_LIB_COMBINED}
    -lpatches
    -ldebug
)

target_compile_options(select_test_iop.elf PRIVATE -O0 -g)

set_target_properties(select_test_iop.elf PROPERTIES
    LINK_FLAGS "-L${CMAKE_BINARY_DIR}"
)

add_dependencies(select_test_iop.elf ps2_drivers)
