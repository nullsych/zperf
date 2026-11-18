# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.20.0)

# zperf is the module under test. In CI it is also the west manifest repository; Zephyr merges
# both entries since they have the same module name.
list(APPEND ZEPHYR_EXTRA_MODULES "${CMAKE_CURRENT_LIST_DIR}/../../..")

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(zperf_smoke)

target_sources(app PRIVATE src/main.c)
