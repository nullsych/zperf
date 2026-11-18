# zperf

[![CI](https://github.com/nullsych/zperf/actions/workflows/ci.yml/badge.svg)](https://github.com/nullsych/zperf/actions/workflows/ci.yml)

`zperf` is a Zephyr RTOS runtime tool analyzer.

It measures per-task execution time in microseconds, using the CPU cycle counter (DWT on Cortex-M) by default (see [Timing resolution](#timing-resolution)). It tracks each task’s accumulated runtime `Total runtime`, including total CPU time spent executing, as well as the maximum continuous execution slice observed within the sampling window!

In addition, it counts context switches per task (`Total Switches`) to reflect scheduling activity and task preemption frequency. The tool aggregates this data over a fixed time interval (typically one second) to provide a system-wide performance snapshot, including `CPU load` distribution across tasks, `Idle time` percentage, and `Kernel` overhead.

⚡️ The result is a simple and lightweight runtime profiling tool that helps analyze scheduling efficiency, task behavior, and overall system utilization on embedded device:

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
# ### Calculations Time slice: 1000549

# ################# CPU LOAD 16% ### IDLE 83% ### KERNEL 0% ################## #
```

## Design Philosophy

Although `zperf` currently targets Zephyr RTOS, you may notice that the internal architecture was intentionally designed with portability in mind. The profiling core is largely decoupled from Zephyr-specific APIs, while RTOS integration is isolated behind a small set of tracing and timing hooks. This approach was chosen to make the tool as generic as possible and to simplify future integration with other RTOSes such as `FreeRTOS`, `ThreadX`, `RTEMS`, or custom schedulers.

The original goal was to build a lightweight profiling framework rather than a `Zephyr-only` utility. However, to keep the scope manageable and validate the design in a real-world environment, development initially focused on Zephyr. As a result, the current implementation serves both as a practical profiler for Zephyr-based applications and as a foundation for future RTOS-agnostic extensions.

## Repository layout

```
.
├── src/                   - source files
├── include/               - public API headers
├── map/                   - RTOS hook map (Zephyr-specific)
├── wrapper/               - Zephyr tracing wrapper
├── cfg/                   - default configuration
├── zephyr/                - Zephyr module descriptor (module.yml)
├── CMakeLists.txt         - module cmake (used by Zephyr module system)
├── Kconfig                - module Kconfig options
├── tests/
│   ├── unit/              - host unit tests of the core (Unity + CTest, fake RTOS)
│   ├── integration/       - Zephyr tests run by twister on native_sim
│   └── west.yml           - west workspace used by CI
├── .github/workflows/     - CI
└── examples/
    └── zperf_nrf_demo/      - example Zephyr application for nRF52
```

## Runtime statistics

### Startup statistics

At the start of the device the tool used inner `k_thread_foreach()` Zephyr feature to iterate over all the threads in the system, passing unique Thread ID and user data into our custom `zperf_enumerate_callback()`. As the result you can trace it's unique `Name`, configured `Priority`, `Stack size`, and `Task Id`:

```
Zephyr OS build v4.4.0-4774-g33ed1c41b918 ***
[00:00:00.396,911] <inf> zperf_startup: Iterating 0 thread in the system. Thread data: 
[00:00:00.396,972] <inf> zperf_startup:       Name        : Task_ZPerf
[00:00:00.396,972] <inf> zperf_startup:       Priority    : 5
[00:00:00.396,972] <inf> zperf_startup:       Stack size  : 1024
[00:00:00.397,003] <inf> zperf_startup:       Task Id     : 0
[00:00:00.397,003] <inf> zperf_startup: Iterating 1 thread in the system. Thread data: 
[00:00:00.397,033] <inf> zperf_startup:       Name        : Task_LED
[00:00:00.397,033] <inf> zperf_startup:       Priority    : 7
[00:00:00.397,033] <inf> zperf_startup:       Stack size  : 1024
[00:00:00.397,064] <inf> zperf_startup:       Task Id     : 1
[00:00:00.397,064] <inf> zperf_startup: Iterating 2 thread in the system. Thread data: 
[00:00:00.397,094] <inf> zperf_startup:       Name        : Task_BLE
[00:00:00.397,094] <inf> zperf_startup:       Priority    : 3
[00:00:00.397,125] <inf> zperf_startup:       Stack size  : 2048
[00:00:00.397,125] <inf> zperf_startup:       Task Id     : 2
[00:00:00.397,125] <inf> zperf_startup: Iterating 3 thread in the system. Thread data: 
[00:00:00.397,155] <inf> zperf_startup:       Name        : bt_tx_processor
[00:00:00.397,186] <inf> zperf_startup:       Priority    : -1
[00:00:00.397,186] <inf> zperf_startup:       Stack size  : 1024
[00:00:00.397,186] <inf> zperf_startup:       Task Id     : 3
[00:00:00.397,216] <inf> zperf_startup: Iterating 4 thread in the system. Thread data: 
[00:00:00.397,247] <inf> zperf_startup:       Name        : BT LW WQ
[00:00:00.397,247] <inf> zperf_startup:       Priority    : 10
[00:00:00.397,247] <inf> zperf_startup:       Stack size  : 1408
[00:00:00.397,277] <inf> zperf_startup:       Task Id     : 4
[00:00:00.397,277] <inf> zperf_startup: Iterating 5 thread in the system. Thread data: 
[00:00:00.397,308] <inf> zperf_startup:       Name        : sysworkq
[00:00:00.397,308] <inf> zperf_startup:       Priority    : -1
[00:00:00.397,308] <inf> zperf_startup:       Stack size  : 1024
[00:00:00.397,338] <inf> zperf_startup:       Task Id     : 5
[00:00:00.397,338] <inf> zperf_startup: Iterating 6 thread in the system. Thread data: 
[00:00:00.397,369] <inf> zperf_startup:       Name        : logging
[00:00:00.397,369] <inf> zperf_startup:       Priority    : 14
[00:00:00.397,399] <inf> zperf_startup:       Stack size  : 768
[00:00:00.397,399] <inf> zperf_startup:       Task Id     : 6
[00:00:00.397,399] <inf> zperf_startup: Iterating 7 thread in the system. Thread data: 
[00:00:00.397,430] <inf> zperf_startup:       Name        : idle
[00:00:00.397,460] <inf> zperf_startup:       Priority    : 15
[00:00:00.397,460] <inf> zperf_startup:       Stack size  : 320
[00:00:00.397,460] <inf> zperf_startup:       Task Id     : 7
[00:00:00.397,491] <inf> zperf_startup: Iterating 8 thread in the system. Thread data: 
[00:00:00.397,521] <inf> zperf_startup:       Name        : main
[00:00:00.397,521] <inf> zperf_startup:       Priority    : 0
[00:00:00.397,521] <inf> zperf_startup:       Stack size  : 1024
[00:00:00.397,521] <inf> zperf_startup:       Task Id     : 8
```

| Field | Description |
|-------|-------------|
| `Name` | Thread name registered at creation (`CONFIG_THREAD_NAME=y` required) |
| `Priority` | Zephyr thread priority; negative values are cooperative, positive are preemptive |
| `Stack size` | Allocated stack space in bytes |
| `Task Id` | `zperf`-assigned iterator index used internally to track the thread throughout its lifetime |

`zperf` keeps its own table that maps each thread to its `Task Id`, so it never modifies thread objects and leaves fields like `custom_data` to the application. Threads created after startup get a `Task Id` on their first run, as long as free slots are left (`CONFIG_ZPERF_USER_TASKS_COUNT` + `CONFIG_ZPERF_SYSTEM_TASKS_COUNT` + idle).

>  The ISR statistics are **paused for now**, it's calculation logic need to be reconsidered.

### Sampling window statistics

During runtime the tool prints window system performance (typically per one second):

```
[00:01:14.311,248] <inf> zperf: # #####################   1 Second System Performance   ###################### #
[00:01:14.311,248] <inf> zperf: #                                                                              #
[00:01:14.311,279] <inf> zperf: #   Task Id         Total runtime         Max runtime        Total Switches    #
[00:01:14.311,309] <inf> zperf: #      0                    488                 488                 1          #
[00:01:14.311,340] <inf> zperf: #      1                     62                  31                 4          #
[00:01:14.311,370] <inf> zperf: #      2                    124                  31                 4          #
[00:01:14.311,401] <inf> zperf: #      3                      0                   0                 0          #
[00:01:14.311,462] <inf> zperf: #      4                      0                   0                 0          #
[00:01:14.311,492] <inf> zperf: #      5                      0                   0                 0          #
[00:01:14.311,523] <inf> zperf: #      6                 162689              162689                 1          #
[00:01:14.311,584] <inf> zperf: #      7                 836823              113282                29          #
[00:01:14.311,614] <inf> zperf: #      8                      0                   0                 0          #
[00:01:14.311,614] <inf> zperf: # ############################################################################ #
[00:01:14.311,645] <inf> zperf: # ### Calculations Time slice: 1000549

[00:01:14.311,645] <inf> zperf: # ################# CPU LOAD 16% ### IDLE 83% ### KERNEL 0% ################## #
```

Each row represents one thread tracked during the sampling window:

| Column | Description |
|--------|-------------|
| `Task Id` | Internal `zperf` iterator index assigned at startup (matches `Task Id` from startup log) |
| `Total runtime` | Accumulated CPU time the thread spent executing during this window, in microseconds |
| `Max runtime` | Longest single continuous execution slice observed within the window, in microseconds |
| `Total Switches` | Number of times the thread was scheduled in during the window |

The footer line `Calculations Time slice` shows the total window duration in microseconds, and the CPU load summary reports `CPU LOAD` (active task time), `IDLE` (idle thread time), and `KERNEL` (window time not booked to any tracked thread, e.g. scheduler overhead or threads that didn't get a free `zperf` slot) as percentages of that time slice. Interrupt time is currently booked to the thread that was interrupted.

### Timing resolution

All values are in microseconds. The clock behind them is chosen with `CONFIG_ZPERF_TIMESTAMP_SOURCE`:

| Option | Clock | Resolution |
|--------|-------|------------|
| `CONFIG_ZPERF_TIMESTAMP_TIMING_FUNCTIONS` (default) | Cycle counter of the Zephyr [timing functions](https://docs.zephyrproject.org/latest/kernel/timing_functions/index.html): DWT `CYCCNT` on Cortex-M | 1 µs while the CPU runs, one tick across sleep |
| `CONFIG_ZPERF_TIMESTAMP_UPTIME_TICKS` | `k_uptime_ticks()` | One system tick, `1 / CONFIG_SYS_CLOCK_TICKS_PER_SEC` (about 30.5 µs on nRF52) |

The cycle counter alone isn't a reliable clock: on Cortex-M it typically stops while the CPU sleeps in the idle thread, and it wraps quickly (about every 67 s at 64 MHz). So zperf advances its clock by the cycles counted since the previous reading and keeps it within the current kernel tick: it never falls behind the tick clock (it catches up after sleep) and never runs a full tick ahead of it. Threads are therefore timed to the microsecond, and only idle periods that include sleep are rounded to ticks. The model lives in the RTOS-agnostic `src/zperf_clock.c` and is covered by unit tests, including simulated sleep and oscillator drift.

With `CONFIG_ZPERF_TIMESTAMP_UPTIME_TICKS`, values in the tables are multiples of the tick (31, 61, 92, ... on nRF52) and a slice shorter than one tick is recorded as 0 or as one tick. Window totals stay accurate in both modes, because consecutive slices share their boundaries and the rounding doesn't accumulate across the window.

The tool relies on Zephyr RTOS tracing hooks to collect runtime statistics with minimal overhead. The tool integrates with the scheduler and interrupt tracing infrastructure through the following user hooks:

* `sys_trace_thread_switched_in_user()` – invoked when a thread is scheduled in. `zperf` records the start timestamp of the task execution period.
* `sys_trace_thread_switched_out_user()` – invoked when a thread is scheduled out. `zperf` calculates the elapsed runtime, updates the task's accumulated execution time, maximum runtime slice, and context switch counters.
* `sys_trace_isr_enter_user()` – called on ISR entry. `zperf` records the start of interrupt execution and temporarily suspends task runtime accounting (**paused for now**).
* `sys_trace_isr_exit_user()` – called on ISR exit. `zperf` measures ISR execution time and restores task runtime accounting (**paused for now**).
* `sys_trace_idle_user()` – called when the system enters the idle thread. This allows `zperf` to accurately track idle time and calculate overall CPU utilization.

By leveraging these RTOS-specific hooks, `zperf` continuously monitors task execution time, context-switch activity, CPU load, idle percentage, and kernel overhead, providing a detailed real-time performance profile of the system.


### Task history

In addition to the per-window statistics, `zperf` maintains a fixed-size circular buffer of raw scheduling events. Each entry captures the thread name, the type of scheduler event, and the uptime timestamp in microseconds at which that event occurred.

The buffer is filled continuously as the scheduler runs and is printed at the end of each sampling window (when `CONFIG_ZPERF_TRACE_HISTORY=y`) or on demand by calling `zperf_trace_history()`. Because the buffer is circular, only the most recent `CONFIG_ZPERF_HISTORY_SIZE` events are retained.

Each row represents one scheduler event:

| Column | Description |
|--------|-------------|
| `Task Name` | Thread name as registered with Zephyr (`CONFIG_THREAD_NAME=y` required) |
| `State` | Scheduler event type: `IN` — thread scheduled in, `OUT` — thread scheduled out, `ISR_IN` — ISR entered, `ISR_OUT` — ISR exited, `IDLE` — idle thread running |
| `Timestamp` | Uptime timestamp in microseconds, from the clock described in [Timing resolution](#timing-resolution) |

```
[00:00:02.273,742] <inf> zperf_history: # ############ Collected History ############ #
[00:00:02.273,742] <inf> zperf_history: #                                             #
[00:00:02.273,773] <inf> zperf_history: # Task Name             State      Timestamp  #
[00:00:02.273,834] <inf> zperf_history: # Task_BLE              OUT          2055878  #
[00:00:02.273,864] <inf> zperf_history: # logging               IN           2055878  #
[00:00:02.273,895] <inf> zperf_history: # logging               OUT          2057526  #
[00:00:02.273,925] <inf> zperf_history: # Task_LED              IN           2057556  #
[00:00:02.273,986] <inf> zperf_history: # Task_LED              OUT          2057587  #
[00:00:02.274,017] <inf> zperf_history: # logging               IN           2057587  #
[00:00:02.274,047] <inf> zperf_history: # logging               OUT          2231140  #
[00:00:02.274,078] <inf> zperf_history: # idle                  IN           2231140  #
[00:00:02.274,139] <inf> zperf_history: # idle                  IDLE         2231140  #
[00:00:02.274,169] <inf> zperf_history: # idle                  IDLE         2251923  #
[00:00:02.274,200] <inf> zperf_history: # idle                  IDLE         2252167  #
[00:00:02.274,230] <inf> zperf_history: # idle                  IDLE         2252167  #
[00:00:02.274,291] <inf> zperf_history: # idle                  IDLE         2252289  #
[00:00:02.274,322] <inf> zperf_history: # idle                  IDLE         2252411  #
[00:00:02.274,353] <inf> zperf_history: # idle                  IDLE         2252563  #
[00:00:02.274,383] <inf> zperf_history: # idle                  IDLE         2252655  #
[00:00:02.274,444] <inf> zperf_history: # idle                  IDLE         2253693  #
[00:00:02.274,475] <inf> zperf_history: # idle                  IDLE         2253906  #
[00:00:02.274,505] <inf> zperf_history: # idle                  IDLE         2254181  #
[00:00:02.274,536] <inf> zperf_history: # idle                  IDLE         2254517  #
[00:00:02.274,597] <inf> zperf_history: # idle                  IDLE         2254883  #
[00:00:02.274,627] <inf> zperf_history: # idle                  IDLE         2255157  #
[00:00:02.274,658] <inf> zperf_history: # idle                  IDLE         2255402  #
[00:00:02.274,688] <inf> zperf_history: # idle                  OUT          2273285  #
[00:00:02.274,749] <inf> zperf_history: # Task_ZPerf            IN           2273315  #
[00:00:02.274,749] <inf> zperf_history: # ########################################### #
```

## Use tool in another Zephyr application

Add the repository root as a Zephyr extra module before `find_package(Zephyr ...)` in your
application `CMakeLists.txt`:

```cmake
list(APPEND ZEPHYR_EXTRA_MODULES /path/to/zperf)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(MyApp)
```

Or pass it from the command line:

```sh
west build -b <board> <app> -- -DEXTRA_ZEPHYR_MODULES=/path/to/zperf
```

Enable the module in your `prj.conf`:

```conf
CONFIG_ZPERF=y
CONFIG_ZPERF_USER_TASKS_COUNT=3
CONFIG_ZPERF_SYSTEM_TASKS_COUNT=5 
```

The module provides its own Zephyr trace hook implementation, so an application
only needs to include the public API:

```c
#include <zperf.h>
```

For real example see `examples/zperf_nrf_demo`.

## Build the demo with Docker (Zephyr environment)

This project can be built inside a pre-configured Zephyr Docker environment. This avoids local toolchain issues and ensures reproducible builds.

### Prerequisites

* Docker installed on your system
* Project cloned locally

### Step 1: Start Docker environment

Run the Zephyr build container and mount your project directory:

```bash
sudo docker run --rm -it \
    --user root \
    -v $(pwd):/workdir \
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

Later, start the saved image:

```sh
docker run --rm -it \
    --user root \
    -v $(pwd):/workdir \
    -w /workdir \
    my-zephyr-build-env
```

### Step 3: Build the application

Build the `zperf_nrf_demo` example for the nRF52 DK from your `zephyr-workspace`:

```bash
west build -b nrf52dk/nrf52832 ../workdir/examples/zperf_nrf_demo -p always
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
cp -r build/ ../workdir/examples/zperf_nrf_demo/
```

## Flash

Now you can move back from Docker to your Host and flash device with [nrfjprog](https://www.nordicsemi.com/Products/Development-tools/nRF-Command-Line-Tools):

```sh
nrfjprog --program examples/zperf_nrf_demo/build/zephyr/zephyr.hex --chiperase --reset
```

## See logs 🦀

Once target plugged-in to your Host machine, connect to it's serial port, for example with my 🦀 `Rust`-written Serial/UART tool [rtio](https://github.com/nullsych/rtio), or with any other UART tool as well:

```
$ rtio -d /dev/ttyACM0
[rtio 15:56:04] Connected to device /dev/ttyACM0, baud rate 115200. Ctrl+T, Q to quit

[00:00:04.276,458] <inf> zperf: # #####################   1 Second System Performance   ###################### #
[00:00:04.276,489] <inf> zperf: #                                                                              #
[00:00:04.276,489] <inf> zperf: #   Task Id         Total runtime         Max runtime        Total Switches    #
[00:00:04.276,519] <inf> zperf: #      0                   1465                1465                 1          #
[00:00:04.276,550] <inf> zperf: #      1                    182                  61                 4          #
[00:00:04.276,580] <inf> zperf: #      2                    182                  61                 4          #
[00:00:04.276,641] <inf> zperf: #      3                      0                   0                 0          #
[00:00:04.276,672] <inf> zperf: #      4                      0                   0                 0          #
[00:00:04.276,702] <inf> zperf: #      5                 412140              253357                 5          #
[00:00:04.276,733] <inf> zperf: #      6                      0                   0                 0          #
[00:00:04.276,763] <inf> zperf: #      7                 587341               99487                 5          #
[00:00:04.276,794] <inf> zperf: #      8                      0                   0                 0          #
[00:00:04.276,794] <inf> zperf: # ############################################################################ #
[00:00:04.276,824] <inf> zperf: # ### Calculations Timeslice: 1001556

[00:00:04.276,855] <inf> zperf: # ################# CPU LOAD 41% ### IDLE 58% ### KERNEL 0% ################## #

[rtio 15:56:06] Quit rtio
```

## Testing

### Unit tests (host)

`tests/unit` tests the zperf core on the host. The core is compiled against stub Zephyr headers and a fake RTOS that owns a simulated clock and drives the same `sys_trace_*_user()` hooks Zephyr calls, so each test describes a schedule and checks what zperf booked: runtime, max slices and switch counters per window, CPU load math, idle detection, the thread table and the task history. Two more tests check that the build fails with a clear message when `CONFIG_TRACING_USER` is off or there are more than 254 tasks.

Only CMake and a C compiler are needed, [Unity](https://github.com/ThrowTheSwitch/Unity) is fetched by CMake. From the repository root:

```sh
cmake -S tests/unit -B build/unit -DZPERF_TEST_SANITIZE=ON   # ASan + UBSan, optional
cmake --build build/unit -j
ctest --test-dir build/unit --output-on-failure
```

* Everything at once: `scripts/run_host_ut.sh` runs the 64-bit build with sanitizers and the 32-bit build (skipped if `gcc-multilib` is missing). Arguments go to `ctest`, e.g. `scripts/run_host_ut.sh -R test_clock`.
* One test suite: `ctest --test-dir build/unit -R test_clock`, or run the binary directly: `build/unit/test_clock`.
* 32-bit build, as in CI (needs `sudo apt install gcc-multilib`):
  ```sh
  cmake -S tests/unit -B build/unit32 -DZPERF_TEST_M32=ON
  cmake --build build/unit32 -j
  ctest --test-dir build/unit32 --output-on-failure
  ```

### Integration tests (Zephyr, native_sim)

`tests/integration/zperf_smoke` runs zperf on a real Zephyr kernel on `native_sim`: a worker thread is busy 2 ms out of every 10 ms, and twister checks the console output for the startup thread list, a CPU load of about 20%, and named history rows for the idle and worker threads. `examples/zperf_nrf_demo/sample.yaml` adds a build-only check of the nRF52 demo.

#### With Docker

In the container from [Build the demo with Docker](#build-the-demo-with-docker-zephyr-environment) (Zephyr in `/zephyr-workspace`, this repository mounted at `/workdir`):

```sh
cd /zephyr-workspace
west twister -T /workdir/tests/integration -T /workdir/examples \
    --integration --inline-logs --outdir /tmp/twister-out
```

This runs the same as CI: both scenarios on `native_sim` and `native_sim/native/64`, plus the nRF52 demo build.

* Only native_sim, faster: `west twister -T /workdir/tests/integration -p native_sim/native/64 --outdir /tmp/twister-out`
* One scenario: add `-s zperf.smoke.uptime_ticks`
* zperf output of a run: `/tmp/twister-out/<platform>/.../handler.log`

Keep `--outdir` outside `/workdir`, otherwise the twister results end up in the repository. The container uses its own Zephyr checkout, which may differ from the revision pinned in `tests/west.yml` for CI.

#### In another Zephyr workspace

For example the one from `tests/west.yml`:

```sh
west twister -T zperf/tests/integration -T zperf/examples --integration --inline-logs
```

### CI

[.github/workflows/ci.yml](.github/workflows/ci.yml) runs on every push to `master` and on pull requests:

* **Unit tests**: 64-bit with ASan + UBSan, and 32-bit.
* **Zephyr**: sets up the workspace from `tests/west.yml` (zperf as the manifest repository, a pinned Zephyr revision and only the modules the demo needs), then runs twister on `native_sim` and `native_sim/native/64` and builds the nRF52 demo.

## Authors

Maintained by [nullsych](https://github.com/nullsych).
