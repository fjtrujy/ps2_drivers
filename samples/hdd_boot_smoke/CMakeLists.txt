add_executable(hdd_boot_smoke.elf hdd_boot_smoke.c)
target_compile_definitions(hdd_boot_smoke.elf PRIVATE PS2_DRIVERS_IMG_FLAVOR=1)
target_link_libraries(hdd_boot_smoke.elf PRIVATE
    "${CMAKE_BINARY_DIR}/${EE_LIB_IMG_COMBINED}"
    -lpatches
    -ldebug
)
add_dependencies(hdd_boot_smoke.elf ps2_drivers_img ps2_drivers_irximg)

# The same universal BOOT.ELF is distributed for every storage transport.
set(HDD_SMOKE_DIR "${CMAKE_CURRENT_BINARY_DIR}/HDD_SMOKE")
set(HDD_SMOKE_ARCHIVE "${CMAKE_CURRENT_BINARY_DIR}/HDD_SMOKE.zip")
file(MAKE_DIRECTORY "${HDD_SMOKE_DIR}")
add_custom_command(
    OUTPUT "${HDD_SMOKE_ARCHIVE}"
    COMMAND ${CMAKE_COMMAND} -E make_directory "${HDD_SMOKE_DIR}"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:ps2_drivers_irximg_bootstrap.elf>
        "${HDD_SMOKE_DIR}/BOOT.ELF"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:hdd_boot_smoke.elf> "${HDD_SMOKE_DIR}/HDD_SMOKE.ELF"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${IRX_IMAGE}" "${HDD_SMOKE_DIR}/ps2_drivers.irximg"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_CURRENT_SOURCE_DIR}/elf_path.ini"
        "${CMAKE_CURRENT_SOURCE_DIR}/probe.txt"
        "${CMAKE_CURRENT_SOURCE_DIR}/README.txt"
        "${HDD_SMOKE_DIR}"
    COMMAND ${CMAKE_COMMAND} -E tar cf "${HDD_SMOKE_ARCHIVE}" --format=zip
        BOOT.ELF HDD_SMOKE.ELF elf_path.ini probe.txt ps2_drivers.irximg README.txt
    WORKING_DIRECTORY "${HDD_SMOKE_DIR}"
    DEPENDS hdd_boot_smoke.elf
        ps2_drivers_irximg_bootstrap.elf
        ps2_drivers_irximg
        "${IRX_IMAGE}"
        "${CMAKE_CURRENT_SOURCE_DIR}/elf_path.ini"
        "${CMAKE_CURRENT_SOURCE_DIR}/probe.txt"
        "${CMAKE_CURRENT_SOURCE_DIR}/README.txt"
    COMMENT "Packaging standalone APA/PFS HDD smoke test"
    VERBATIM
)
add_custom_target(hdd_smoke_package DEPENDS "${HDD_SMOKE_ARCHIVE}")
