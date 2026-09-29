add_executable(cacheio_compare_sample.elf cacheio_compare_sample.c)

target_include_directories(cacheio_compare_sample.elf PRIVATE
    ${CMAKE_SOURCE_DIR}/include
)

target_compile_definitions(cacheio_compare_sample.elf PRIVATE PS2_DRIVERS_IMG_FLAVOR=1)

target_link_libraries(cacheio_compare_sample.elf
    ${CMAKE_BINARY_DIR}/${EE_LIB_IMG_COMBINED}
    -lpatches
)

target_compile_options(cacheio_compare_sample.elf PRIVATE -O2 -g)

add_dependencies(cacheio_compare_sample.elf ps2_drivers_img ps2_drivers_irximg)

add_custom_command(
    TARGET cacheio_compare_sample.elf
    POST_BUILD
    COMMAND ${CMAKE_COMMAND}
        -DNM=${CMAKE_NM}
        -DARTIFACT=$<TARGET_FILE:cacheio_compare_sample.elf>
        -P ${CMAKE_SOURCE_DIR}/cmake/check_no_embedded_irx.cmake
    COMMENT "Checking cacheio sample for embedded IRX payloads"
    VERBATIM
)

# Refresh the runtime image even when only an IRX changes and the ELF does not relink.
add_custom_target(cacheio_compare_runtime ALL
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${IRX_IMAGE}
        $<TARGET_FILE_DIR:cacheio_compare_sample.elf>/ps2_drivers.irximg
    DEPENDS cacheio_compare_sample.elf ps2_drivers_irximg
    VERBATIM
)
