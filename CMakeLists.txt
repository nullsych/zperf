cmake_minimum_required(VERSION 3.20.0)

# Include main Zephyr SDK
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

project(NrfSysPerfDemo)

FILE(GLOB MAIN_SOURCES ./src/*.c)
FILE(GLOB CMN_SOURCES ./src/common/Leds/*.c)
FILE(GLOB PM_SOURCES ./src/PwrMgt/*.c)

# Add SysPerf library
add_subdirectory(lib/SysPerf)

# Include project directories
target_include_directories(
    app
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src/common
    ${CMAKE_CURRENT_SOURCE_DIR}/src/common/Leds
    ${CMAKE_CURRENT_SOURCE_DIR}/src/common/PwrMgt
# Include directories for SysPerf
    ${CMAKE_CURRENT_SOURCE_DIR}/lib/SysPerf/include
    ${CMAKE_CURRENT_SOURCE_DIR}/lib/SysPerf/cfg
)

# Add application sources
target_sources(
    app
    PRIVATE
    ${MAIN_SOURCES}
    ${CMN_SOURCES}
    ${PM_SOURCES}
)

# Add SysPerf build directory to link directories
target_link_directories(
    app
    PRIVATE
    ${CMAKE_BINARY_DIR}/lib/SysPerf
)

# Link SysPerf library
target_link_libraries(
    app
    PRIVATE
    SysPerf
)
