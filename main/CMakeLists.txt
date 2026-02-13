set(COMP_SRCS
    ime-embarcados-lib.cpp
)

# If hvcc output exists under main/hvcc/c, include all generated C sources.
set(HVCC_DIR "${CMAKE_CURRENT_LIST_DIR}/hvcc/c")
if(EXISTS ${HVCC_DIR})
    file(GLOB_RECURSE HVCC_SRCS "${HVCC_DIR}/*.c" "${HVCC_DIR}/*.cpp")
    list(APPEND COMP_SRCS ${HVCC_SRCS})
    set(HVCC_INC ${HVCC_DIR})
    list(APPEND COMP_SRCS hv_bridge.cpp)
endif()

idf_component_register(
    SRCS ${COMP_SRCS}
    INCLUDE_DIRS "." ${HVCC_INC}
)

if(EXISTS ${HVCC_DIR})
    target_compile_definitions(${COMPONENT_LIB} PRIVATE HV_ENABLED=1)
endif()