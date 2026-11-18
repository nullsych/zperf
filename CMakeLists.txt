if(CONFIG_ZPERF)
    set(ZPERF_INCLUDE_DIRS
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/wrapper
        ${CMAKE_CURRENT_SOURCE_DIR}/cfg
        ${CMAKE_CURRENT_SOURCE_DIR}/map
    )

    zephyr_library_named(zperf)

    zephyr_library_sources(
        src/zperf.c
        src/zperf_history.c
        src/zperf_clock.c
        wrapper/rtos_wrapper.c
        map/rtos_hooks.c
    )

    zephyr_library_include_directories(
        ${ZPERF_INCLUDE_DIRS}
    )

    zephyr_include_directories(
        ${ZPERF_INCLUDE_DIRS}
    )
endif()
