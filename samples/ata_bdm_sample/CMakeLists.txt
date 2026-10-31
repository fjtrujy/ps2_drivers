add_executable(ata_bdm_sample.elf ata_bdm_sample.c)
target_include_directories(ata_bdm_sample.elf PRIVATE ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(ata_bdm_sample.elf PRIVATE
    "${CMAKE_BINARY_DIR}/${EE_LIB_COMBINED}"
    -lpatches
)
add_dependencies(ata_bdm_sample.elf ps2_drivers)

add_executable(ata_bdm_img_sample.elf ata_bdm_sample.c)
target_compile_definitions(ata_bdm_img_sample.elf PRIVATE PS2_DRIVERS_IMG_FLAVOR=1)
target_include_directories(ata_bdm_img_sample.elf PRIVATE ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(ata_bdm_img_sample.elf PRIVATE
    "${CMAKE_BINARY_DIR}/${EE_LIB_IMG_COMBINED}"
    -lpatches
)
add_dependencies(ata_bdm_img_sample.elf ps2_drivers_img ps2_drivers_irximg)

add_executable(ata_bdm_r3z_smoke.elf ata_bdm_sample.c)
target_compile_definitions(ata_bdm_r3z_smoke.elf PRIVATE
    PS2_DRIVERS_IMG_FLAVOR=1 PS2_DRIVERS_R3Z_SMOKE=1)
target_include_directories(ata_bdm_r3z_smoke.elf PRIVATE
    ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(ata_bdm_r3z_smoke.elf PRIVATE
    "${CMAKE_BINARY_DIR}/${EE_LIB_IMG_COMBINED}"
    -lpatches -ldebug
)
add_dependencies(ata_bdm_r3z_smoke.elf ps2_drivers_img ps2_drivers_irximg)

add_executable(ata_bdm_probe_launcher.elf ata_bdm_probe_launcher.c)
target_include_directories(ata_bdm_probe_launcher.elf PRIVATE ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(ata_bdm_probe_launcher.elf PRIVATE
    "${CMAKE_BINARY_DIR}/${EE_LIB_COMBINED}"
    -lelf-loader
    -lpatches
)
add_dependencies(ata_bdm_probe_launcher.elf ps2_drivers)

# Emulates wLaunchELF-R3Z's ata0: launch argument while the actual ELF is
# loaded from an ATA-backed BDM mass0: mount.
add_executable(ata_bdm_r3z_probe_launcher.elf ata_bdm_probe_launcher.c)
target_compile_definitions(ata_bdm_r3z_probe_launcher.elf PRIVATE
    PS2_DRIVERS_TEST_R3Z_ATA_HANDOFF=1)
target_include_directories(ata_bdm_r3z_probe_launcher.elf PRIVATE
    ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(ata_bdm_r3z_probe_launcher.elf PRIVATE
    "${CMAKE_BINARY_DIR}/${EE_LIB_COMBINED}"
    -lelf-loader
    -lpatches
)
add_dependencies(ata_bdm_r3z_probe_launcher.elf ps2_drivers)

# Ready-to-copy on-device reproduction for the actual R3Z file browser.
set(ATA_R3Z_SMOKE_DIR "${CMAKE_CURRENT_BINARY_DIR}/ATA_R3Z_SMOKE")
set(ATA_R3Z_SMOKE_ZIP "${CMAKE_CURRENT_BINARY_DIR}/ATA_R3Z_SMOKE.zip")
file(MAKE_DIRECTORY "${ATA_R3Z_SMOKE_DIR}")
add_custom_command(
    OUTPUT "${ATA_R3Z_SMOKE_ZIP}"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:ps2_drivers_irximg_bootstrap.elf>
        "${ATA_R3Z_SMOKE_DIR}/BOOT.ELF"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:ata_bdm_r3z_smoke.elf>
        "${ATA_R3Z_SMOKE_DIR}/ATA_TEST.ELF"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${IRX_IMAGE}" "${ATA_R3Z_SMOKE_DIR}/ps2_drivers.irximg"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_CURRENT_SOURCE_DIR}/README_R3Z.txt"
        "${ATA_R3Z_SMOKE_DIR}/README_R3Z.txt"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_CURRENT_SOURCE_DIR}/elf_path_r3z.ini"
        "${ATA_R3Z_SMOKE_DIR}/elf_path.ini"
    COMMAND ${CMAKE_COMMAND} -E tar cf "${ATA_R3Z_SMOKE_ZIP}"
        --format=zip
        BOOT.ELF ATA_TEST.ELF elf_path.ini ps2_drivers.irximg README_R3Z.txt
    WORKING_DIRECTORY "${ATA_R3Z_SMOKE_DIR}"
    DEPENDS ps2_drivers_irximg_bootstrap.elf ata_bdm_r3z_smoke.elf
        ps2_drivers_irximg "${IRX_IMAGE}"
        "${CMAKE_CURRENT_SOURCE_DIR}/elf_path_r3z.ini"
        "${CMAKE_CURRENT_SOURCE_DIR}/README_R3Z.txt"
    COMMENT "Packaging standalone wLaunchELF-R3Z ATA/exFAT smoke test"
    VERBATIM
)
add_custom_target(ata_r3z_smoke_package DEPENDS "${ATA_R3Z_SMOKE_ZIP}")
