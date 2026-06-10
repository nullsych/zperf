set(NRF_PERF_DEMO_SRC_DIR "${NRF_PERF_DEMO_APP_DIR}/src")

target_include_directories(
    app
    PRIVATE
    ${NRF_PERF_DEMO_SRC_DIR}
    ${NRF_PERF_DEMO_SRC_DIR}/common
    ${NRF_PERF_DEMO_SRC_DIR}/common/Leds
    ${NRF_PERF_DEMO_SRC_DIR}/PwrMgt
)

target_sources(
    app
    PRIVATE
    ${NRF_PERF_DEMO_SRC_DIR}/main.c
    ${NRF_PERF_DEMO_SRC_DIR}/BleTask.c
    ${NRF_PERF_DEMO_SRC_DIR}/LedTask.c
    ${NRF_PERF_DEMO_SRC_DIR}/SysPerfTask.c
    ${NRF_PERF_DEMO_SRC_DIR}/common/Leds/Leds.c
    ${NRF_PERF_DEMO_SRC_DIR}/PwrMgt/PwrMgt.c
)
