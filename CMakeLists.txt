cmake_minimum_required(VERSION 3.13)

# Set the toolchain file before project()
if(NOT CMAKE_TOOLCHAIN_FILE)
    if(DEFINED ENV{PS2DEV})
        set(CMAKE_TOOLCHAIN_FILE "$ENV{PS2DEV}/share/ps2dev.cmake" CACHE FILEPATH "Toolchain file")
    else()
        message(FATAL_ERROR "PS2DEV environment variable is not set. Please set it to your PS2DEV installation path.")
    endif()
endif()

project(ps2_drivers VERSION 1.0.0 LANGUAGES C)

# Options
option(BUILD_SAMPLES "Build sample projects" ON)

# Check if PS2SDK is set (should be done by toolchain file)
if(NOT DEFINED PS2SDK)
    message(FATAL_ERROR "PS2SDK is not defined. Make sure the toolchain file is loaded correctly.")
endif()

# Set the bin2c tool path
if(NOT DEFINED BIN2C)
    set(BIN2C "${PS2SDK}/bin/bin2c")
endif()

# Set output library names
set(EE_LIB_IMPL "libps2_drivers_impl.a")
set(EE_LIB_COMBINED "libps2_drivers.a")

# Compiler flags (matching the Makefile)
add_compile_options(-Werror)
include_directories(${CMAKE_SOURCE_DIR}/include)

# ============================================================================
# Function: add_irx_source
# Converts an IRX file to a C source file using bin2c
# ============================================================================
function(add_irx_source irx_name output_var)
    set(irx_path "${PS2SDK}/iop/irx/${irx_name}.irx")
    set(output_c "${CMAKE_CURRENT_BINARY_DIR}/${irx_name}_irx.c")

    add_custom_command(
        OUTPUT ${output_c}
        COMMAND ${BIN2C} ${irx_path} ${output_c} ${irx_name}_irx
        DEPENDS ${irx_path}
        COMMENT "Converting ${irx_name}.irx to C source"
        VERBATIM
    )

    set(${output_var} ${output_c} PARENT_SCOPE)
endfunction()

# ============================================================================
# Function: add_driver_objects
# Compiles a driver source file multiple times with different defines
# ============================================================================
function(add_driver_objects driver_name)
    # Parse the function names from remaining arguments
    set(functions ${ARGN})
    set(driver_objs "")

    foreach(func IN LISTS functions)
        set(obj_name "${func}_ps2_${driver_name}_driver")
        add_library(${obj_name} OBJECT "${CMAKE_SOURCE_DIR}/src/ps2_${driver_name}_driver.c")
        target_compile_definitions(${obj_name} PRIVATE "F_${func}_ps2_${driver_name}_driver")
        target_include_directories(${obj_name} PRIVATE "${CMAKE_SOURCE_DIR}/include")
        list(APPEND driver_objs $<TARGET_OBJECTS:${obj_name}>)
    endforeach()

    set(${driver_name}_DRIVER_OBJECTS ${driver_objs} PARENT_SCOPE)
endfunction()

# ============================================================================
# Generate IRX source files
# ============================================================================
add_irx_source(sio2man SIO2MAN_IRX)
add_irx_source(iomanX IOMANX_IRX)
add_irx_source(fileXio FILEXIO_IRX)
add_irx_source(mcman MCMAN_IRX)
add_irx_source(mcserv MCSERV_IRX)
add_irx_source(bdm BDM_IRX)
add_irx_source(bdmfs_fatfs BDMFS_FATFS_IRX)
add_irx_source(usbd USBD_IRX)
add_irx_source(usbmass_bd USBMASS_BD_IRX)
add_irx_source(mx4sio_bd MX4SIO_BD_IRX)
add_irx_source(cdfs CDFS_IRX)
add_irx_source(ps2dev9 PS2DEV9_IRX)
add_irx_source(ps2atad PS2ATAD_IRX)
add_irx_source(ps2hdd PS2HDD_IRX)
add_irx_source(ps2fs PS2FS_IRX)
add_irx_source(mtapman MTAPMAN_IRX)
add_irx_source(padman PADMAN_IRX)
add_irx_source(libsd LIBSD_IRX)
add_irx_source(audsrv AUDSRV_IRX)
add_irx_source(poweroff POWEROFF_IRX)
add_irx_source(ps2mouse PS2MOUSE_IRX)
add_irx_source(ps2kbd PS2KBD_IRX)
add_irx_source(ps2cam PS2CAM_IRX)
add_irx_source(netman NETMAN_IRX)
add_irx_source(smap SMAP_IRX)

set(IRX_SOURCES
    ${SIO2MAN_IRX}
    ${IOMANX_IRX}
    ${FILEXIO_IRX}
    ${MCMAN_IRX}
    ${MCSERV_IRX}
    ${BDM_IRX}
    ${BDMFS_FATFS_IRX}
    ${USBD_IRX}
    ${USBMASS_BD_IRX}
    ${MX4SIO_BD_IRX}
    ${CDFS_IRX}
    ${PS2DEV9_IRX}
    ${PS2ATAD_IRX}
    ${PS2HDD_IRX}
    ${PS2FS_IRX}
    ${MTAPMAN_IRX}
    ${PADMAN_IRX}
    ${LIBSD_IRX}
    ${AUDSRV_IRX}
    ${POWEROFF_IRX}
    ${PS2MOUSE_IRX}
    ${PS2KBD_IRX}
    ${PS2CAM_IRX}
    ${NETMAN_IRX}
    ${SMAP_IRX}
)

# ============================================================================
# Generate driver objects
# ============================================================================
# SIO2MAN driver
add_driver_objects(sio2man internals init deinit)

# FileXio driver
add_driver_objects(fileXio internals init deinit)

# Memory card driver
add_driver_objects(memcard internals init deinit)

# BDM driver
add_driver_objects(bdm internals init deinit)

# USBD driver
add_driver_objects(usbd internals init deinit)

# USB driver
add_driver_objects(usb internals init deinit)

# MX4SIO driver
add_driver_objects(mx4sio internals init deinit)

# CDFS driver
add_driver_objects(cdfs internals init deinit)

# DEV9 driver
add_driver_objects(dev9 internals init deinit)

# HDD driver
add_driver_objects(hdd
    internals
    init
    deinit
    mount_hdd_partition
    mount_current_partition
    umount_hdd_partition
    umount_current_partition
)

# Filesystem driver
add_driver_objects(filesystem
    waitUntilDeviceIsReady
    rootDevicePath
    getBootDeviceID
    __internal_deinit
    deinit
    init
    __internal_deinit_only_boot
    deinit_only_boot
    init_only_boot
)

# Joystick driver
add_driver_objects(joystick internals init deinit)

# Audio driver
add_driver_objects(audio internals init deinit)

# Poweroff driver
add_driver_objects(poweroff internals init deinit)

# Mouse driver
add_driver_objects(mouse internals init deinit)

# Keyboard driver
add_driver_objects(keyboard internals init deinit)

# Camera driver
add_driver_objects(camera internals init deinit)

# Netman driver
add_driver_objects(netman internals init deinit)

# SMAP driver
add_driver_objects(smap internals init deinit)

# EEIP driver
add_driver_objects(eeip internals init deinit)

# IOPIP driver
add_driver_objects(iopip internals init deinit)

# ============================================================================
# Build the implementation library
# ============================================================================
add_library(ps2_drivers_impl STATIC
    ${IRX_SOURCES}
    ${sio2man_DRIVER_OBJECTS}
    ${fileXio_DRIVER_OBJECTS}
    ${memcard_DRIVER_OBJECTS}
    ${bdm_DRIVER_OBJECTS}
    ${usbd_DRIVER_OBJECTS}
    ${usb_DRIVER_OBJECTS}
    ${mx4sio_DRIVER_OBJECTS}
    ${cdfs_DRIVER_OBJECTS}
    ${dev9_DRIVER_OBJECTS}
    ${hdd_DRIVER_OBJECTS}
    ${filesystem_DRIVER_OBJECTS}
    ${joystick_DRIVER_OBJECTS}
    ${audio_DRIVER_OBJECTS}
    ${poweroff_DRIVER_OBJECTS}
    ${mouse_DRIVER_OBJECTS}
    ${keyboard_DRIVER_OBJECTS}
    ${camera_DRIVER_OBJECTS}
    ${netman_DRIVER_OBJECTS}
    ${smap_DRIVER_OBJECTS}
    ${eeip_DRIVER_OBJECTS}
    ${iopip_DRIVER_OBJECTS}
)

set_target_properties(ps2_drivers_impl PROPERTIES
    OUTPUT_NAME "ps2_drivers_impl"
    ARCHIVE_OUTPUT_NAME "ps2_drivers_impl"
)

# ============================================================================
# Combine with PS2SDK libraries to create final library
# ============================================================================
# List of PS2SDK libraries to combine
set(PS2SDK_LIBS
    "${PS2SDK}/ee/lib/libfileXio.a"
    "${PS2SDK}/ee/lib/libmtap.a"
    "${PS2SDK}/ee/lib/libpadx.a"
    "${PS2SDK}/ee/lib/libaudsrv.a"
    "${PS2SDK}/ee/lib/libpoweroff.a"
    "${PS2SDK}/ee/lib/libmouse.a"
    "${PS2SDK}/ee/lib/libkbd.a"
    "${PS2SDK}/ee/lib/libps2cam.a"
    "${PS2SDK}/ee/lib/libnetman.a"
    "${PS2SDK}/ee/lib/libps2ip.a"
)

# Create the combined library using ar with MRI script
# Generate the MRI script content with all libraries to combine
set(MRI_SCRIPT_CONTENT "create ${CMAKE_CURRENT_BINARY_DIR}/${EE_LIB_COMBINED}\n")
set(MRI_SCRIPT_CONTENT "${MRI_SCRIPT_CONTENT}addlib $<TARGET_FILE:ps2_drivers_impl>\n")
foreach(lib ${PS2SDK_LIBS})
    set(MRI_SCRIPT_CONTENT "${MRI_SCRIPT_CONTENT}addlib ${lib}\n")
endforeach()
set(MRI_SCRIPT_CONTENT "${MRI_SCRIPT_CONTENT}save\nend\n")

# Write MRI script at configure time (generator expressions evaluated at build time via file(GENERATE))
file(GENERATE OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/mri_script.txt CONTENT "${MRI_SCRIPT_CONTENT}")

add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${EE_LIB_COMBINED}
    COMMAND ${CMAKE_COMMAND} -E rm -f ${CMAKE_CURRENT_BINARY_DIR}/${EE_LIB_COMBINED}
    COMMAND ${CMAKE_AR} -M < ${CMAKE_CURRENT_BINARY_DIR}/mri_script.txt
    DEPENDS ps2_drivers_impl ${PS2SDK_LIBS} ${CMAKE_CURRENT_BINARY_DIR}/mri_script.txt
    COMMENT "Combining libraries into ${EE_LIB_COMBINED}"
)

add_custom_target(ps2_drivers ALL
    DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/${EE_LIB_COMBINED}
)

# ============================================================================
# Install targets
# ============================================================================
install(
    FILES ${CMAKE_CURRENT_BINARY_DIR}/${EE_LIB_COMBINED}
    DESTINATION "${PS2SDK}/ports/lib"
)

install(
    DIRECTORY ${CMAKE_SOURCE_DIR}/include/
    DESTINATION "${PS2SDK}/ports/include"
    FILES_MATCHING PATTERN "*.h"
)

# ============================================================================
# Include samples if enabled
# ============================================================================
if(BUILD_SAMPLES)
    add_subdirectory(samples)
endif()

# ============================================================================
# Print configuration summary
# ============================================================================
message(STATUS "")
message(STATUS "ps2_drivers configuration:")
message(STATUS "  Version: ${PROJECT_VERSION}")
message(STATUS "  Build samples: ${BUILD_SAMPLES}")
message(STATUS "  Output library: ${EE_LIB_COMBINED}")
message(STATUS "  Install prefix: ${PS2SDK}/ports")
message(STATUS "")
