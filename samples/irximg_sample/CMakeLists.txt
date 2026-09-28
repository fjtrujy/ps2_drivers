add_executable(irximg_sample.elf irximg_sample.c)
add_executable(irximg_embedded_reference.elf irximg_sample.c)

target_include_directories(irximg_sample.elf PRIVATE
    ${CMAKE_SOURCE_DIR}/include
)
target_include_directories(irximg_embedded_reference.elf PRIVATE
    ${CMAKE_SOURCE_DIR}/include
)

target_compile_definitions(irximg_sample.elf PRIVATE PS2_DRIVERS_IMG_FLAVOR=1)

target_link_libraries(irximg_sample.elf
    ${CMAKE_BINARY_DIR}/${EE_LIB_IMG_COMBINED}
    -lpatches
)
target_link_libraries(irximg_embedded_reference.elf
    ${CMAKE_BINARY_DIR}/${EE_LIB_COMBINED}
    -lpatches
)

target_compile_options(irximg_sample.elf PRIVATE -O0 -g)
target_compile_options(irximg_embedded_reference.elf PRIVATE -O0 -g)

add_dependencies(irximg_sample.elf ps2_drivers_img ps2_drivers_irximg)
add_dependencies(irximg_embedded_reference.elf ps2_drivers)

add_custom_command(
    TARGET irximg_sample.elf
    POST_BUILD
    COMMAND ${CMAKE_COMMAND}
        -DNM=${CMAKE_NM}
        -DARTIFACT=$<TARGET_FILE:irximg_sample.elf>
        -P ${CMAKE_SOURCE_DIR}/cmake/check_no_embedded_irx.cmake
    COMMENT "Checking image-flavor ELF for embedded IRX payloads"
    VERBATIM
)
