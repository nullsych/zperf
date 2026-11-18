# SPDX-License-Identifier: Apache-2.0
#
# Host unit tests for the zperf core. The core is built against stub Zephyr headers and a fake
# RTOS (tests/unit/fake), so these tests need no Zephyr checkout and run in a few seconds:
#
#   cmake -S tests/unit -B build/unit
#   cmake --build build/unit
#   ctest --test-dir build/unit --output-on-failure

cmake_minimum_required(VERSION 3.20)
project(zperf_unit_tests LANGUAGES C)

option(ZPERF_TEST_SANITIZE "Build with AddressSanitizer and UndefinedBehaviorSanitizer" OFF)
option(ZPERF_TEST_M32 "Build 32-bit binaries (needs gcc-multilib)" OFF)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)

if(ZPERF_TEST_M32)
  add_compile_options(-m32)
  add_link_options(-m32)
endif()

if(ZPERF_TEST_SANITIZE)
  add_compile_options(-fsanitize=address,undefined -fno-sanitize-recover=all -fno-omit-frame-pointer)
  add_link_options(-fsanitize=address,undefined)
endif()

include(FetchContent)
FetchContent_Declare(
  unity
  GIT_REPOSITORY https://github.com/ThrowTheSwitch/Unity.git
  GIT_TAG v2.6.1
  GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(unity)

set(ZPERF_ROOT ${CMAKE_CURRENT_LIST_DIR}/../..)

# Kconfig values the tests are written against: 3 user + 2 system + 1 idle = 6 tasks, and a
# small history so wrap-around is easy to reach
set(ZPERF_TEST_CONFIG
  CONFIG_TRACING_USER=1
  CONFIG_ZPERF_TRACE_MAIN_STAT=1
  CONFIG_ZPERF_TRACE_CPU_LOAD=1
  CONFIG_ZPERF_TRACE_HISTORY=1
  CONFIG_ZPERF_RTOS_HAS_SYSTEM_TASKS=1
  CONFIG_ZPERF_SYSTEM_TASKS_COUNT=2
  CONFIG_ZPERF_HISTORY_SIZE=8
  CONFIG_THREAD_MAX_NAME_LEN=32
)

add_library(zperf_test_env INTERFACE)
target_include_directories(zperf_test_env INTERFACE
  ${CMAKE_CURRENT_LIST_DIR}
  ${CMAKE_CURRENT_LIST_DIR}/stubs
  ${CMAKE_CURRENT_LIST_DIR}/fake
  ${ZPERF_ROOT}/src
  ${ZPERF_ROOT}/include
  ${ZPERF_ROOT}/wrapper
  ${ZPERF_ROOT}/cfg
  ${ZPERF_ROOT}/map
)
target_compile_definitions(zperf_test_env INTERFACE ${ZPERF_TEST_CONFIG})
target_compile_options(zperf_test_env INTERFACE -Wall -Wextra)

enable_testing()

function(zperf_unit_test name)
  add_executable(${name}
    ${name}.c
    fake/fake_rtos.c
    ${ZPERF_ROOT}/map/rtos_hooks.c
  )
  target_compile_definitions(${name} PRIVATE CONFIG_ZPERF_USER_TASKS_COUNT=3)
  target_link_libraries(${name} PRIVATE zperf_test_env unity)
  add_test(NAME ${name} COMMAND ${name})
endfunction()

zperf_unit_test(test_window_stats)
zperf_unit_test(test_cpu_load)
zperf_unit_test(test_idle)
zperf_unit_test(test_threads)
zperf_unit_test(test_history)

# The clock is RTOS-agnostic: tested on its own, without the fake RTOS
add_executable(test_clock test_clock.c ${ZPERF_ROOT}/src/zperf_clock.c)
target_include_directories(test_clock PRIVATE ${ZPERF_ROOT}/include)
target_compile_options(test_clock PRIVATE -Wall -Wextra)
target_link_libraries(test_clock PRIVATE unity)
add_test(NAME test_clock COMMAND test_clock)

# Build-time guards: these targets must fail to compile, with the expected message.
# They are excluded from the default build and compiled by the tests themselves.
add_library(compile_fail_too_many_tasks OBJECT EXCLUDE_FROM_ALL ${ZPERF_ROOT}/src/zperf.c)
target_compile_definitions(compile_fail_too_many_tasks PRIVATE CONFIG_ZPERF_USER_TASKS_COUNT=300)
target_link_libraries(compile_fail_too_many_tasks PRIVATE zperf_test_env)

add_library(compile_fail_no_tracing_user OBJECT EXCLUDE_FROM_ALL ${ZPERF_ROOT}/map/rtos_hooks.c)
target_compile_definitions(compile_fail_no_tracing_user PRIVATE CONFIG_ZPERF_USER_TASKS_COUNT=3)
target_link_libraries(compile_fail_no_tracing_user PRIVATE zperf_test_env)
target_compile_options(compile_fail_no_tracing_user PRIVATE -UCONFIG_TRACING_USER)

function(zperf_compile_fail_test target expected_message)
  add_test(NAME ${target}
    COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target ${target})
  set_tests_properties(${target} PROPERTIES
    PASS_REGULAR_EXPRESSION "${expected_message}"
    # Both run the build tool in this build directory
    RESOURCE_LOCK zperf_build_dir)
endfunction()

zperf_compile_fail_test(compile_fail_too_many_tasks "zperf supports up to 254 tasks")
zperf_compile_fail_test(compile_fail_no_tracing_user "zperf requires CONFIG_TRACING_USER=y")
