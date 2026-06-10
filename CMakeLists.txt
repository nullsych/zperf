if(CONFIG_NRF_PERF)
    set(NRF_PERF_INCLUDE_DIRS
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/wrapper
        ${CMAKE_CURRENT_SOURCE_DIR}/cfg
        ${CMAKE_CURRENT_SOURCE_DIR}/map
    )

    zephyr_library_named(nrf_perf)

    zephyr_library_sources(
        src/nrf_perf.c
        src/nrf_perf_history.c
        wrapper/nrf_perf_zephyr_wrapper.c
        map/rtos_hooks.c
    )

    zephyr_library_include_directories(
        ${NRF_PERF_INCLUDE_DIRS}
    )

    zephyr_include_directories(
        ${NRF_PERF_INCLUDE_DIRS}
    )
endif()
