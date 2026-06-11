set(ZPERF_NRF_DEMO_SRC_DIR "${ZPERF_NRF_DEMO_APP_DIR}/src")

target_include_directories(
    app
    PRIVATE
    ${ZPERF_NRF_DEMO_SRC_DIR}
    ${ZPERF_NRF_DEMO_SRC_DIR}/common
    ${ZPERF_NRF_DEMO_SRC_DIR}/common/Leds
    ${ZPERF_NRF_DEMO_SRC_DIR}/PwrMgt
)

target_sources(
    app
    PRIVATE
    ${ZPERF_NRF_DEMO_SRC_DIR}/main.c
    ${ZPERF_NRF_DEMO_SRC_DIR}/BleTask.c
    ${ZPERF_NRF_DEMO_SRC_DIR}/LedTask.c
    ${ZPERF_NRF_DEMO_SRC_DIR}/ZPerfTask.c
    ${ZPERF_NRF_DEMO_SRC_DIR}/common/Leds/Leds.c
    ${ZPERF_NRF_DEMO_SRC_DIR}/PwrMgt/PwrMgt.c
)
