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

set(INSTALL_LIB_DIR "${PS2SDK}/ports/lib" CACHE PATH "Installation directory for libraries")
set(INSTALL_INC_DIR "${PS2SDK}/ports/include" CACHE PATH "Installation directory for headers")
set(INSTALL_PKGCONFIG_DIR "${PS2SDK}/ports/lib/pkgconfig" CACHE PATH "Installation directory for pkgconfig (.pc) files")

# Prepare git metadata
set(CUR_GIT_TAG v0.0.0)
set(CUR_GIT_COMMIT_HASH "")
set(CUR_GIT_COMMIT_TIMESTAMP "")
set(CUR_GIT_REFS "")
set(CUR_GIT_REMOTE_URL "")
find_package(Git)
if(GIT_FOUND)
    if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/.git)
        execute_process(COMMAND ${GIT_EXECUTABLE} describe --abbrev=0 --tags
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        OUTPUT_VARIABLE TMP_GIT_TAG
        RESULT_VARIABLE TMP_RES
        OUTPUT_STRIP_TRAILING_WHITESPACE)
        execute_process(COMMAND ${GIT_EXECUTABLE} show --no-patch --format=%H
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        OUTPUT_VARIABLE TMP_GIT_COMMIT_HASH
        RESULT_VARIABLE TMP_RES
        OUTPUT_STRIP_TRAILING_WHITESPACE)
        if(TMP_RES EQUAL "0")
            set(CUR_GIT_COMMIT_HASH ${TMP_GIT_COMMIT_HASH})
        endif()
        execute_process(COMMAND ${GIT_EXECUTABLE} show --no-patch --format=%cd --date=unix
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        OUTPUT_VARIABLE TMP_GIT_COMMIT_TIMESTAMP
        RESULT_VARIABLE TMP_RES
        OUTPUT_STRIP_TRAILING_WHITESPACE)
        if(TMP_RES EQUAL "0")
            set(CUR_GIT_COMMIT_TIMESTAMP ${TMP_GIT_COMMIT_TIMESTAMP})
        endif()
        execute_process(COMMAND ${GIT_EXECUTABLE} show --no-patch "--format=%(decorate:prefix=,suffix=,tag=,separator=%x2C,pointer=%x2C)"
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        OUTPUT_VARIABLE TMP_GIT_REFS
        RESULT_VARIABLE TMP_RES
        OUTPUT_STRIP_TRAILING_WHITESPACE)
        if(TMP_RES EQUAL "0")
            set(CUR_GIT_REFS ${TMP_GIT_REFS})
        endif()
        execute_process(COMMAND ${GIT_EXECUTABLE} config --get remote.origin.url
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        OUTPUT_VARIABLE TMP_GIT_REMOTE_URL
        RESULT_VARIABLE TMP_RES
        OUTPUT_STRIP_TRAILING_WHITESPACE)
        if(TMP_RES EQUAL "0")
            set(CUR_GIT_REMOTE_URL ${TMP_GIT_REMOTE_URL})
        endif()
    endif()
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
function(add_irx_source irx_filename irx_symbol output_var)
    set(irx_path "${PS2SDK}/iop/irx/${irx_filename}")
    set(output_c "${CMAKE_CURRENT_BINARY_DIR}/${irx_symbol}.c")

    add_custom_command(
        OUTPUT ${output_c}
        COMMAND ${BIN2C} ${irx_path} ${output_c} ${irx_symbol}
        DEPENDS ${irx_path}
        COMMENT "Converting ${irx_filename} to C source"
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
# Generate IRX source files from the canonical manifest
# ============================================================================
set(IRX_MANIFEST "${CMAKE_SOURCE_DIR}/irx/manifest.txt")
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "${IRX_MANIFEST}")
file(STRINGS "${IRX_MANIFEST}" IRX_MANIFEST_LINES)

set(IRX_SOURCES "")
set(IRX_INPUTS "")

foreach(irx_manifest_line IN LISTS IRX_MANIFEST_LINES)
    string(STRIP "${irx_manifest_line}" irx_manifest_line)
    if(irx_manifest_line STREQUAL "" OR irx_manifest_line MATCHES "^#")
        continue()
    endif()

    string(REPLACE "|" ";" irx_manifest_fields "${irx_manifest_line}")
    list(LENGTH irx_manifest_fields irx_manifest_field_count)
    if(NOT irx_manifest_field_count EQUAL 4)
        message(FATAL_ERROR "Invalid IRX manifest entry: ${irx_manifest_line}")
    endif()

    list(GET irx_manifest_fields 1 irx_filename)
    list(GET irx_manifest_fields 2 irx_symbol)

    add_irx_source("${irx_filename}" "${irx_symbol}" generated_irx_source)
    list(APPEND IRX_SOURCES "${generated_irx_source}")
    list(APPEND IRX_INPUTS "${PS2SDK}/iop/irx/${irx_filename}")
endforeach()

if(NOT IRX_SOURCES)
    message(FATAL_ERROR "IRX manifest contains no modules: ${IRX_MANIFEST}")
endif()

# ============================================================================
# Build the host-side IRX image tool and versioned companion image
# ============================================================================
find_program(PS2_DRIVERS_HOST_C_COMPILER NAMES cc gcc clang)
if(NOT PS2_DRIVERS_HOST_C_COMPILER)
    message(FATAL_ERROR "A host C compiler (cc, gcc, or clang) is required to build ps2_irximg_tool")
endif()

set(IRXIMG_TOOL_DIR "${CMAKE_CURRENT_BINARY_DIR}/host-tools")
set(IRXIMG_TOOL "${IRXIMG_TOOL_DIR}/ps2_irximg_tool")
set(IRXIMG_TOOL_SOURCE "${CMAKE_SOURCE_DIR}/tools/ps2_irximg.c")
set(IRX_IMAGE "${CMAKE_CURRENT_BINARY_DIR}/ps2_drivers.irximg")
set(IRX_IMAGE_DETERMINISM "${CMAKE_CURRENT_BINARY_DIR}/ps2_drivers.determinism.irximg")

add_custom_command(
    OUTPUT "${IRXIMG_TOOL}"
    COMMAND ${CMAKE_COMMAND} -E make_directory "${IRXIMG_TOOL_DIR}"
    COMMAND ${PS2_DRIVERS_HOST_C_COMPILER}
        -std=c99
        -O2
        -Wall
        -Wextra
        -Werror
        "${IRXIMG_TOOL_SOURCE}"
        -o "${IRXIMG_TOOL}"
    DEPENDS "${IRXIMG_TOOL_SOURCE}"
    COMMENT "Building host ps2_irximg_tool"
    VERBATIM
)

add_custom_command(
    OUTPUT "${IRX_IMAGE}"
    COMMAND "${IRXIMG_TOOL}" pack "${IRX_MANIFEST}" "${PS2SDK}/iop/irx" "${IRX_IMAGE}"
    COMMAND "${IRXIMG_TOOL}" validate "${IRX_MANIFEST}" "${IRX_IMAGE}"
    DEPENDS "${IRXIMG_TOOL}" "${IRX_MANIFEST}" ${IRX_INPUTS}
    COMMENT "Packing ps2_drivers.irximg"
    VERBATIM
)

add_custom_target(ps2_drivers_irximg ALL
    DEPENDS "${IRX_IMAGE}"
)

add_custom_target(ps2_drivers_irximg_check
    COMMAND "${IRXIMG_TOOL}" pack "${IRX_MANIFEST}" "${PS2SDK}/iop/irx" "${IRX_IMAGE_DETERMINISM}"
    COMMAND "${IRXIMG_TOOL}" validate "${IRX_MANIFEST}" "${IRX_IMAGE_DETERMINISM}"
    COMMAND ${CMAKE_COMMAND} -E compare_files "${IRX_IMAGE}" "${IRX_IMAGE_DETERMINISM}"
    COMMAND ${CMAKE_COMMAND} -E remove -f "${IRX_IMAGE_DETERMINISM}"
    DEPENDS "${IRX_IMAGE}" "${IRXIMG_TOOL}" "${IRX_MANIFEST}" ${IRX_INPUTS}
    COMMENT "Checking deterministic IRX image generation"
    VERBATIM
)

add_custom_target(ps2_drivers_irximg_inspect
    COMMAND "${IRXIMG_TOOL}" inspect "${IRX_MANIFEST}" "${IRX_IMAGE}"
    DEPENDS "${IRX_IMAGE}" "${IRXIMG_TOOL}" "${IRX_MANIFEST}"
    COMMENT "Inspecting ps2_drivers.irximg"
    USES_TERMINAL
    VERBATIM
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
add_driver_objects(eeip
    internals
    init
    deinit
    configure_eeip_network
    eeip_network_config_default_dhcp
    eeip_get_current_config
)

# IOPIP driver
add_driver_objects(iopip
    internals
    init
    deinit
    configure_iopip_network
    iopip_network_config_default
    iopip_get_current_config
)

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
    "${PS2SDK}/ee/lib/libps2ips.a"
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
# Prepare pkg-config metadata file
# ============================================================================
set(PCKEY_NAME ${PROJECT_NAME})
set(PCKEY_DESCRIPTION "Convenience library for ps2sdk I/O drivers")
set(PCKEY_VERSION ${PROJECT_VERSION})
set(PCKEY_REQUIRES "")
set(PCKEY_LIBS "-lps2_drivers")
set(PCKEY_CFLAGS "")
set(PS2_DRIVERS_PC ${CMAKE_CURRENT_BINARY_DIR}/ps2_drivers.pc)
configure_file( ${CMAKE_CURRENT_SOURCE_DIR}/ps2_drivers.pc.cmakein
    ${PS2_DRIVERS_PC} @ONLY)

# ============================================================================
# Install targets
# ============================================================================
if(NOT SKIP_INSTALL_LIBRARIES AND NOT SKIP_INSTALL_ALL)
    install(
        FILES ${CMAKE_CURRENT_BINARY_DIR}/${EE_LIB_COMBINED}
        DESTINATION "${INSTALL_LIB_DIR}"
    )
endif()

if(NOT SKIP_INSTALL_HEADERS AND NOT SKIP_INSTALL_ALL)
    install(
        DIRECTORY ${CMAKE_SOURCE_DIR}/include/
        DESTINATION "${INSTALL_INC_DIR}"
        FILES_MATCHING PATTERN "*.h"
    )
endif()

if(NOT SKIP_INSTALL_FILES AND NOT SKIP_INSTALL_ALL )
    install(
        FILES ${PS2_DRIVERS_PC}
        DESTINATION "${INSTALL_PKGCONFIG_DIR}"
    )
endif()

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
message(STATUS "  IRX image: ${IRX_IMAGE}")
message(STATUS "  Install prefix: ${PS2SDK}/ports")
message(STATUS "")
