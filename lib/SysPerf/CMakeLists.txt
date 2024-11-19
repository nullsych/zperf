# Automatically collect all source files in the src directory
file(GLOB SYSPERF_SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.c
)

# Define cfg files
set(SYSPERF_CFG
    ${CMAKE_CURRENT_SOURCE_DIR}/cfg
)

# Define header files
set(SYSPERF_INCLUDE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

set(SYS_PERF_LIB_NAME
    "SysPerf"
)

zephyr_library_named(${SYS_PERF_LIB_NAME})
zephyr_library_sources(${SYSPERF_SRC})
zephyr_library_include_directories(
    ${SYSPERF_INCLUDE}
    ${SYSPERF_CFG}
)