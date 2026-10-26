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

add_executable(ata_bdm_probe_launcher.elf ata_bdm_probe_launcher.c)
target_include_directories(ata_bdm_probe_launcher.elf PRIVATE ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(ata_bdm_probe_launcher.elf PRIVATE
    "${CMAKE_BINARY_DIR}/${EE_LIB_COMBINED}"
    -lelf-loader
    -lpatches
)
add_dependencies(ata_bdm_probe_launcher.elf ps2_drivers)
