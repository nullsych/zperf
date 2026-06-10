# nrf_perf

`nrf_perf` is a Zephyr runtime analyzer.

It measures per-task execution behavior in microseconds with high-resolution timing. It tracks each task’s accumulated runtime `Total runtime`, including total CPU time spent executing, as well as the maximum continuous execution slice observed within the sampling window!

In addition, it counts context switches per task (`Total Switches`) to reflect scheduling activity and task preemption frequency. The library aggregates this data over a fixed time interval (typically one second) to provide a system-wide performance snapshot, including `CPU load` distribution across tasks, `Idle time` percentage, and `Kernel` overhead.

⚡️ The result is a simple and precise runtime profiling tool that helps analyze scheduling efficiency, task behavior, and overall system utilization on embedded device:

```
# #####################   1 Second System Performance   ###################### #
#                                                                              #
#   Task Id         Total runtime         Max runtime        Total Switches    #
#      0                    458                 458                 1          #
#      1                     61                  61                 4          #
#      2                    154                  61                 4          #
#      3                      0                   0                 0          #
#      4                      0                   0                 0          #
#      5                      0                   0                 0          #
#      6                 162598              144135                 3          #
#      7                 836944              107727                27          #
#      8                      0                   0                 0          #
# ############################################################################ #
# ### Calculations Timeslice: 1000549

# ################# CPU LOAD 16% ### IDLE 83% ### KERNEL 0% ################## #
```

## Design Philosophy

Although `nrf_perf` currently targets Zephyr RTOS, you may notice that the internal architecture was intentionally designed with portability in mind. The profiling core is largely decoupled from Zephyr-specific APIs, while RTOS integration is isolated behind a small set of tracing and timing hooks. This approach was chosen to make the library as generic as possible and to simplify future integration with other RTOSes such as `FreeRTOS`, `ThreadX`, `RTEMS`, or custom schedulers.

The original goal was to build a lightweight profiling framework rather than a `Zephyr-only` utility. However, to keep the scope manageable and validate the design in a real-world environment, development initially focused on Zephyr. As a result, the current implementation serves both as a practical profiler for Zephyr-based applications and as a foundation for future RTOS-agnostic extensions.

## Repository layout

```
.
├── src/                   - library source files
├── include/               - public API headers
├── map/                   - RTOS hook map (Zephyr-specific)
├── wrapper/               - Zephyr tracing wrapper
├── cfg/                   - default configuration
├── zephyr/                - Zephyr module descriptor (module.yml)
├── CMakeLists.txt         - module cmake (used by Zephyr module system)
├── Kconfig                - module Kconfig options
└── examples/
    └── nrfperf_demo/      - example Zephyr application for nRF52
```

## Runtime statistics

### Startup statistics

At the start of the device library used inner `k_thread_foreach()` Zephyr feature to iterate over all the threads in the system, passing unique Thread ID and user data into our custom `nrf_perf_enumerate_callback()`. As the result you can trace it's unique `Name`, configured `Priority`, `Stack size`, and `Custom data`:

```
Zephyr OS build v4.4.0-4774-g33ed1c41b918 ***
[00:00:00.396,911] <inf> main: Perform post-scheduler initializations for nrf_perf
[00:00:00.396,911] <inf> nrf_perf_zephyr_wrapper: Iterating 0 thread in the system. Thread data: 
[00:00:00.396,972] <inf> nrf_perf_zephyr_wrapper:       Name        : Task_NrfPerf
[00:00:00.396,972] <inf> nrf_perf_zephyr_wrapper:       Priority    : 5
[00:00:00.396,972] <inf> nrf_perf_zephyr_wrapper:       Stack size  : 1024
[00:00:00.397,003] <inf> nrf_perf_zephyr_wrapper:       Custom data : 0
[00:00:00.397,003] <inf> nrf_perf_zephyr_wrapper: Iterating 1 thread in the system. Thread data: 
[00:00:00.397,033] <inf> nrf_perf_zephyr_wrapper:       Name        : Task_LED
[00:00:00.397,033] <inf> nrf_perf_zephyr_wrapper:       Priority    : 7
[00:00:00.397,033] <inf> nrf_perf_zephyr_wrapper:       Stack size  : 1024
[00:00:00.397,064] <inf> nrf_perf_zephyr_wrapper:       Custom data : 1
[00:00:00.397,064] <inf> nrf_perf_zephyr_wrapper: Iterating 2 thread in the system. Thread data: 
[00:00:00.397,094] <inf> nrf_perf_zephyr_wrapper:       Name        : Task_BLE
[00:00:00.397,094] <inf> nrf_perf_zephyr_wrapper:       Priority    : 3
[00:00:00.397,125] <inf> nrf_perf_zephyr_wrapper:       Stack size  : 2048
[00:00:00.397,125] <inf> nrf_perf_zephyr_wrapper:       Custom data : 2
[00:00:00.397,125] <inf> nrf_perf_zephyr_wrapper: Iterating 3 thread in the system. Thread data: 
[00:00:00.397,155] <inf> nrf_perf_zephyr_wrapper:       Name        : bt_tx_processor
[00:00:00.397,186] <inf> nrf_perf_zephyr_wrapper:       Priority    : -1
[00:00:00.397,186] <inf> nrf_perf_zephyr_wrapper:       Stack size  : 1024
[00:00:00.397,186] <inf> nrf_perf_zephyr_wrapper:       Custom data : 3
[00:00:00.397,216] <inf> nrf_perf_zephyr_wrapper: Iterating 4 thread in the system. Thread data: 
[00:00:00.397,247] <inf> nrf_perf_zephyr_wrapper:       Name        : BT LW WQ
[00:00:00.397,247] <inf> nrf_perf_zephyr_wrapper:       Priority    : 10
[00:00:00.397,247] <inf> nrf_perf_zephyr_wrapper:       Stack size  : 1408
[00:00:00.397,277] <inf> nrf_perf_zephyr_wrapper:       Custom data : 4
[00:00:00.397,277] <inf> nrf_perf_zephyr_wrapper: Iterating 5 thread in the system. Thread data: 
[00:00:00.397,308] <inf> nrf_perf_zephyr_wrapper:       Name        : sysworkq
[00:00:00.397,308] <inf> nrf_perf_zephyr_wrapper:       Priority    : -1
[00:00:00.397,308] <inf> nrf_perf_zephyr_wrapper:       Stack size  : 1024
[00:00:00.397,338] <inf> nrf_perf_zephyr_wrapper:       Custom data : 5
[00:00:00.397,338] <inf> nrf_perf_zephyr_wrapper: Iterating 6 thread in the system. Thread data: 
[00:00:00.397,369] <inf> nrf_perf_zephyr_wrapper:       Name        : logging
[00:00:00.397,369] <inf> nrf_perf_zephyr_wrapper:       Priority    : 14
[00:00:00.397,399] <inf> nrf_perf_zephyr_wrapper:       Stack size  : 768
[00:00:00.397,399] <inf> nrf_perf_zephyr_wrapper:       Custom data : 6
[00:00:00.397,399] <inf> nrf_perf_zephyr_wrapper: Iterating 7 thread in the system. Thread data: 
[00:00:00.397,430] <inf> nrf_perf_zephyr_wrapper:       Name        : idle
[00:00:00.397,460] <inf> nrf_perf_zephyr_wrapper:       Priority    : 15
[00:00:00.397,460] <inf> nrf_perf_zephyr_wrapper:       Stack size  : 320
[00:00:00.397,460] <inf> nrf_perf_zephyr_wrapper:       Custom data : 7
[00:00:00.397,491] <inf> nrf_perf_zephyr_wrapper: Iterating 8 thread in the system. Thread data: 
[00:00:00.397,521] <inf> nrf_perf_zephyr_wrapper:       Name        : main
[00:00:00.397,521] <inf> nrf_perf_zephyr_wrapper:       Priority    : 0
[00:00:00.397,521] <inf> nrf_perf_zephyr_wrapper:       Stack size  : 1024
[00:00:00.397,521] <inf> nrf_perf_zephyr_wrapper:       Custom data : 8
```

We use `Custom data` for assigning a task-iterator to track the Task later.

>  The ISR statistics are **paused for now**, it's calculation logic need to be reconsidered.

### Sampling window statistics

During runtime library prints window system performance (typically per one second):

```
[00:01:14.311,248] <inf> nrf_perf: # #####################   1 Second System Performance   ###################### #
[00:01:14.311,248] <inf> nrf_perf: #                                                                              #
[00:01:14.311,279] <inf> nrf_perf: #   Task Id         Total runtime         Max runtime        Total Switches    #
[00:01:14.311,309] <inf> nrf_perf: #      0                    488                 488                 1          #
[00:01:14.311,340] <inf> nrf_perf: #      1                     62                  31                 4          #
[00:01:14.311,370] <inf> nrf_perf: #      2                    124                  31                 4          #
[00:01:14.311,401] <inf> nrf_perf: #      3                      0                   0                 0          #
[00:01:14.311,462] <inf> nrf_perf: #      4                      0                   0                 0          #
[00:01:14.311,492] <inf> nrf_perf: #      5                      0                   0                 0          #
[00:01:14.311,523] <inf> nrf_perf: #      6                 162689              162689                 1          #
[00:01:14.311,584] <inf> nrf_perf: #      7                 836823              113282                29          #
[00:01:14.311,614] <inf> nrf_perf: #      8                      0                   0                 0          #
[00:01:14.311,614] <inf> nrf_perf: # ############################################################################ #
[00:01:14.311,645] <inf> nrf_perf: # ### Calculations Timeslice: 1000549

[00:01:14.311,645] <inf> nrf_perf: # ################# CPU LOAD 16% ### IDLE 83% ### KERNEL 0% ################## #
```

The tool relies on Zephyr RTOS tracing hooks to collect runtime statistics with minimal overhead. The library integrates with the scheduler and interrupt tracing infrastructure through the following user hooks:

* `sys_trace_thread_switched_in_user()` – invoked when a thread is scheduled in. `nrf_perf` records the start timestamp of the task execution period.
* `sys_trace_thread_switched_out_user()` – invoked when a thread is scheduled out. `nrf_perf` calculates the elapsed runtime, updates the task's accumulated execution time, maximum runtime slice, and context switch counters.
* `sys_trace_isr_enter_user()` – called on ISR entry. `nrf_perf` records the start of interrupt execution and temporarily suspends task runtime accounting (**paused for now**).
* `sys_trace_isr_exit_user()` – called on ISR exit. `nrf_perf` measures ISR execution time and restores task runtime accounting (**paused for now**).
* `sys_trace_idle_user()` – called when the system enters the idle thread. This allows `nrf_perf` to accurately track idle time and calculate overall CPU utilization.

By leveraging these RTOS-specific hooks, `nrf_perf` continuously monitors task execution time in microseconds, context-switch activity, interrupt processing time, CPU load, idle percentage, and kernel overhead, providing a detailed real-time performance profile of the system.


## Use tool in another Zephyr application

Add the repository root as a Zephyr extra module before `find_package(Zephyr ...)` in your
application `CMakeLists.txt`:

```cmake
list(APPEND ZEPHYR_EXTRA_MODULES /path/to/nrf_perf)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(MyApp)
```

Or pass it from the command line:

```sh
west build -b <board> <app> -- -DEXTRA_ZEPHYR_MODULES=/path/to/nrf_perf
```

Enable the module in your `prj.conf`:

```conf
CONFIG_NRF_PERF=y
CONFIG_NRF_PERF_USER_TASKS_COUNT=3
CONFIG_NRF_PERF_SYSTEM_TASKS_COUNT=5 
```

The module provides its own Zephyr trace hook implementation, so an application
only needs to include the public API:

```c
#include <nrf_perf.h>
```

For real example see `examples/nrfperf_demo`.

## Build the demo with Docker (Zephyr environment)

This project can be built inside a pre-configured Zephyr Docker environment. This avoids local toolchain issues and ensures reproducible builds.

### Prerequisites

* Docker installed on your system
* Project cloned locally

### Step 1: Start Docker environment

Run the Zephyr build container and mount your project directory:

```bash
docker run --rm -it \
    --user root \
    -v (pwd):/workdir \
    -w /workdir \
    zephyrprojectrtos/zephyr-build:latest
```

This will start an interactive shell inside a fully configured Zephyr build environment.

### Step 2: Initialize workspace (first time only)

If this is a fresh workspace, run in root of the Docker:

```
cd /
sudo mkdir zephyr-workspace
cd zephyr-workspace
west init .
west update
west zephyr-export
```

#### Saving the initialized workspace (optional)

The initialization step (`west init`, `west update`) may take some time. If you plan to reuse the same environment frequently, you can save the container state as a custom image.

First, open another terminal and find the running container:

```sh
docker ps
```

E.g.:

``` sh
CONTAINER ID   IMAGE                                   COMMAND                  CREATED         STATUS         PORTS      NAMES
f4229357be37   zephyrprojectrtos/zephyr-build:latest   "/home/entrypoint.sh…"   2 minutes ago   Up 2 minutes   5900/tcp   flamboyant_morse
```

And commit your Zephyr container:

```
docker commit f4229357be37 my-zephyr-build-env
```

### Step 3: Build the application

Build the `nrfperf_demo` example for the nRF52 DK from your `zephyr-workspace`:

```bash
west build -b nrf52dk/nrf52832 ../workdir/examples/nrfperf_demo -p always
```

> Note: `-p always` is used instead of `--pristine` to ensure a clean rebuild.

The result:

```
...
...
-- west build: building application
[1/391] Preparing syscall dependency handling

[3/391] Generating include/generated/zephyr/version.h
-- Zephyr version: 4.4.99 (/zephyr-workspace/zephyr), build: v4.4.0-4774-g33ed1c41b918
[391/391] Linking C executable zephyr/zephyr.elf
Memory region         Used Size  Region Size  %age Used
           FLASH:      165796 B       512 KB     31.62%
             RAM:       45080 B        64 KB     68.79%
        IDT_LIST:           0 B        32 KB      0.00%
Generating files from /zephyr-workspace/build/zephyr/zephyr.elf for board: nrf52dk/nrf52832
```

The output `build/` is in your `/zephyr-workspace/`, so copy it into the root of the project to flash:

```sh
cp -r build/ ../workdir/examples/nrfperf_demo/
```

## Flash

Now you can move back from Docker to your Host and flash device with [nrfjprog](https://www.nordicsemi.com/Products/Development-tools/nRF-Command-Line-Tools):

```sh
nrfjprog --program examples/nrfperf_demo/build/zephyr/zephyr.hex --chiperase --reset
```

## See logs 🦀

Once target plugged-in to your Host machine, connect to it's serial port, for example with my 🦀 `Rust`-written Serial/UART tool [rtio](https://github.com/nullsych/rtio), or with any other UART tool as well:

```
$ rtio -d /dev/ttyACM0
[rtio 15:56:04] Connected to device /dev/ttyACM0, baud rate 115200. Ctrl+T, Q to quit

[00:01:57.335,083] <inf> nrf_perf: # #####################   1 Second System Performance   ###################### #
[00:01:57.335,083] <inf> nrf_perf: #                                                                              #
[00:01:57.335,113] <inf> nrf_perf: #   Task Id         Total runtime         Max runtime        Total Switches    #
[00:01:57.335,144] <inf> nrf_perf: #      0                    458                 458                 1          #
[00:01:57.335,174] <inf> nrf_perf: #      1                     61                  61                 4          #
[00:01:57.335,205] <inf> nrf_perf: #      2                    154                  61                 4          #
[00:01:57.335,266] <inf> nrf_perf: #      3                      0                   0                 0          #
[00:01:57.335,296] <inf> nrf_perf: #      4                      0                   0                 0          #
[00:01:57.335,327] <inf> nrf_perf: #      5                      0                   0                 0          #
[00:01:57.335,357] <inf> nrf_perf: #      6                 162598              144135                 3          #
[00:01:57.335,418] <inf> nrf_perf: #      7                 836944              107727                27          #
[00:01:57.335,449] <inf> nrf_perf: #      8                      0                   0                 0          #
[00:01:57.335,449] <inf> nrf_perf: # ############################################################################ #
[00:01:57.335,479] <inf> nrf_perf: # ### Calculations Timeslice: 1000549

[00:01:57.335,479] <inf> nrf_perf: # ################# CPU LOAD 16% ### IDLE 83% ### KERNEL 0% ################## #

[00:01:58.335,632] <inf> nrf_perf: # #####################   1 Second System Performance   ###################### #
[00:01:58.335,632] <inf> nrf_perf: #                                                                              #
[00:01:58.335,662] <inf> nrf_perf: #   Task Id         Total runtime         Max runtime        Total Switches    #
[00:01:58.335,693] <inf> nrf_perf: #      0                    489                 489                 1          #
[00:01:58.335,723] <inf> nrf_perf: #      1                     61                  61                 4          #
[00:01:58.335,754] <inf> nrf_perf: #      2                    152                  62                 4          #
[00:01:58.335,784] <inf> nrf_perf: #      3                      0                   0                 0          #
[00:01:58.335,845] <inf> nrf_perf: #      4                      0                   0                 0          #
[00:01:58.335,876] <inf> nrf_perf: #      5                      0                   0                 0          #
[00:01:58.335,906] <inf> nrf_perf: #      6                 162749              124359                 3          #
[00:01:58.335,968] <inf> nrf_perf: #      7                 836792              104920                25          #
[00:01:58.335,998] <inf> nrf_perf: #      8                      0                   0                 0          #
[00:01:58.335,998] <inf> nrf_perf: # ############################################################################ #
[00:01:58.335,998] <inf> nrf_perf: # ### Calculations Timeslice: 1000550

[00:01:58.336,029] <inf> nrf_perf: # ################# CPU LOAD 16% ### IDLE 83% ### KERNEL 0% ################## #

[rtio 15:56:06] Quit rtio
```

## Authors

Maintained by [nullsych](https://github.com/nullsych).
