file(STRINGS "${MANIFEST}" manifest_lines)

foreach(manifest_line IN LISTS manifest_lines)
    string(STRIP "${manifest_line}" manifest_line)
    if(manifest_line STREQUAL "" OR manifest_line MATCHES "^#")
        continue()
    endif()

    string(REPLACE "|" ";" fields "${manifest_line}")
    list(GET fields 1 filename)

    if(filename STREQUAL "cacheio.irx")
        set(source "${CACHEIO_IRX}")
    else()
        set(source "${PS2SDK_IRX_DIR}/${filename}")
    endif()

    if(NOT EXISTS "${source}")
        message(FATAL_ERROR "Missing IRX image input: ${source}")
    endif()
    file(COPY_FILE "${source}" "${OUTPUT_DIR}/${filename}" ONLY_IF_DIFFERENT)
endforeach()
