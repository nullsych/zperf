# zperf demo

This Zephyr application shows how to use the `zperf` module from an app.
Its `CMakeLists.txt` registers the module with `ZEPHYR_EXTRA_MODULES`, and
`prj.conf` enables `CONFIG_ZPERF`.

Build:

```sh
west build -b nrf52dk_nrf52832 examples/zperf_nrf_demo
```
