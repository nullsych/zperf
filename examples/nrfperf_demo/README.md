# nRF nrf_perf demo

This Zephyr application shows how to use the `lib/nrf_perf` module from an app.
Its `CMakeLists.txt` registers the module with `ZEPHYR_EXTRA_MODULES`, and
`prj.conf` enables `CONFIG_NRF_PERF`.

Build:

```sh
west build -b nrf52dk_nrf52832 examples/nrfperf_demo
```
