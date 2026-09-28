if(NOT DEFINED NM OR NOT DEFINED ARTIFACT)
    message(FATAL_ERROR "NM and ARTIFACT are required")
endif()

execute_process(
    COMMAND "${NM}" "${ARTIFACT}"
    RESULT_VARIABLE nm_result
    OUTPUT_VARIABLE nm_output
    ERROR_VARIABLE nm_error
)

if(NOT nm_result EQUAL 0)
    message(FATAL_ERROR "nm failed for ${ARTIFACT}: ${nm_error}")
endif()

string(REGEX MATCH "(^|\n)[^\n]*[ \\t][BDR][ \\t]+[A-Za-z0-9_]+_irx(\n|$)" embedded_irx "${nm_output}")
if(embedded_irx)
    message(FATAL_ERROR "Embedded IRX payload symbol found in ${ARTIFACT}: ${embedded_irx}")
endif()

string(REGEX MATCH "(^|\n)[^\n]*[ \\t][BDR][ \\t]+size_[A-Za-z0-9_]+_irx(\n|$)" embedded_irx_size "${nm_output}")
if(embedded_irx_size)
    message(FATAL_ERROR "Embedded IRX size symbol found in ${ARTIFACT}: ${embedded_irx_size}")
endif()

message(STATUS "No embedded IRX payload symbols found in ${ARTIFACT}")
