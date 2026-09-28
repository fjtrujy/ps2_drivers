# select_test_ee CMakeLists.txt — minimal select() reproducer (EE-side path)

add_executable(select_test_ee.elf select_test_ee.c)

target_include_directories(select_test_ee.elf PRIVATE
    ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(select_test_ee.elf
    ${CMAKE_BINARY_DIR}/${EE_LIB_COMBINED}
    -lpatches
    -ldebug
)

target_compile_options(select_test_ee.elf PRIVATE -O0 -g)

set_target_properties(select_test_ee.elf PROPERTIES
    LINK_FLAGS "-L${CMAKE_BINARY_DIR}"
)

add_dependencies(select_test_ee.elf ps2_drivers)
