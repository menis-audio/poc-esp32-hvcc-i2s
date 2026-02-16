cmake_minimum_required(VERSION 3.16)

# Consume HVCC sources placed under main/hvcc by the generator
set(HVCC_OUT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/hvcc")

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