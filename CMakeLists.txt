cmake_minimum_required(VERSION 3.20.0)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

project(NrfSysPerf)

FILE(GLOB main_sources ./src/*.c)
FILE(GLOB common_sources_leds ./src/common/Leds/*.c)
FILE(GLOB sysperf_sources ./lib/SysPerf/src/*.c)

# Include project's header files
include_directories(
    ./src/
    ./src/common/
    ./src/common/Leds/
    ./lib/SysPerf/src
)

target_sources(
    app
    PRIVATE
    ${main_sources}
    ${common_sources_leds}
    ${sysperf_sources}
)
