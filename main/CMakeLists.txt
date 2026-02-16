cmake_minimum_required(VERSION 3.16)

# Regenerate HVCC sources from Pure Data patch at configure-time
set(HVCC_PD "${CMAKE_CURRENT_SOURCE_DIR}/test.pd")
set(HVCC_OUT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/hvcc")

# Optional: provide additional search paths for hvcc abstractions (semicolon-separated)
set(HVCC_SEARCH_PATHS "" CACHE STRING "Extra search paths for hvcc (e.g. /path/to/heavylib;/another/path)")

find_program(HVCC_EXECUTABLE NAMES hvcc)
if(HVCC_EXECUTABLE AND EXISTS "${HVCC_PD}")
    message(STATUS "HVCC: regenerating sources from ${HVCC_PD}")
    file(REMOVE_RECURSE "${HVCC_OUT_DIR}")
    file(MAKE_DIRECTORY "${HVCC_OUT_DIR}")

    set(HVCC_P_ARGS "")
    if(HVCC_SEARCH_PATHS)
        foreach(P ${HVCC_SEARCH_PATHS})
            list(APPEND HVCC_P_ARGS -p "${P}")
        endforeach()
        message(STATUS "HVCC: using search paths: ${HVCC_SEARCH_PATHS}")
    endif()

    execute_process(
        COMMAND ${HVCC_EXECUTABLE} "${HVCC_PD}" -o "${HVCC_OUT_DIR}" ${HVCC_P_ARGS}
        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
        RESULT_VARIABLE HVCC_RET
        OUTPUT_VARIABLE HVCC_OUT
        ERROR_VARIABLE HVCC_ERR
    )
    if(NOT HVCC_RET EQUAL 0)
        message(FATAL_ERROR "HVCC generation failed (code ${HVCC_RET}).\nOutput: ${HVCC_OUT}\nError: ${HVCC_ERR}")
    endif()
elseif(NOT HVCC_EXECUTABLE)
    message(WARNING "HVCC executable not found on PATH; skipping hvcc generation.")
elseif(NOT EXISTS "${HVCC_PD}")
    message(WARNING "HVCC: ${HVCC_PD} not found. Skipping code generation.")
endif()

# Collect generated HVCC sources dynamically to avoid missing files when hvcc output changes
file(GLOB HVCC_SOURCES
    "${HVCC_OUT_DIR}/c/*.c"
    "${HVCC_OUT_DIR}/c/*.cpp"
)

if(NOT HVCC_SOURCES)
    message(WARNING "HVCC: No sources found in ${HVCC_OUT_DIR}/c. Build will proceed with app sources only.")
endif()

idf_component_register(
    SRCS
        "poc_esp32_hvcc_i2s.c"
        ${HVCC_SOURCES}
    INCLUDE_DIRS 
        "."
        "hvcc/c"
    REQUIRES driver
)
