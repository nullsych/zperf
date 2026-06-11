# zperf

`zperf` is a Zephyr RTOS runtime tool analyzer.

It measures per-task execution behavior in microseconds with high-resolution timing. It tracks each task’s accumulated runtime `Total runtime`, including total CPU time spent executing, as well as the maximum continuous execution slice observed within the sampling window!

In addition, it counts context switches per task (`Total Switches`) to reflect scheduling activity and task preemption frequency. The tool aggregates this data over a fixed time interval (typically one second) to provide a system-wide performance snapshot, including `CPU load` distribution across tasks, `Idle time` percentage, and `Kernel` overhead.

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
└── examples/
    └── zperf_nrf_demo/      - example Zephyr application for nRF52
```

## Runtime statistics

### Startup statistics

At the start of the device the tool used inner `k_thread_foreach()` Zephyr feature to iterate over all the threads in the system, passing unique Thread ID and user data into our custom `zperf_enumerate_callback()`. As the result you can trace it's unique `Name`, configured `Priority`, `Stack size`, and `Custom data`:

```
Zephyr OS build v4.4.0-4774-g33ed1c41b918 ***
[00:00:00.396,911] <inf> zperf_startup: Iterating 0 thread in the system. Thread data: 
[00:00:00.396,972] <inf> zperf_startup:       Name        : Task_ZPerf
[00:00:00.396,972] <inf> zperf_startup:       Priority    : 5
[00:00:00.396,972] <inf> zperf_startup:       Stack size  : 1024
[00:00:00.397,003] <inf> zperf_startup:       Custom data : 0
[00:00:00.397,003] <inf> zperf_startup: Iterating 1 thread in the system. Thread data: 
[00:00:00.397,033] <inf> zperf_startup:       Name        : Task_LED
[00:00:00.397,033] <inf> zperf_startup:       Priority    : 7
[00:00:00.397,033] <inf> zperf_startup:       Stack size  : 1024
[00:00:00.397,064] <inf> zperf_startup:       Custom data : 1
[00:00:00.397,064] <inf> zperf_startup: Iterating 2 thread in the system. Thread data: 
[00:00:00.397,094] <inf> zperf_startup:       Name        : Task_BLE
[00:00:00.397,094] <inf> zperf_startup:       Priority    : 3
[00:00:00.397,125] <inf> zperf_startup:       Stack size  : 2048
[00:00:00.397,125] <inf> zperf_startup:       Custom data : 2
[00:00:00.397,125] <inf> zperf_startup: Iterating 3 thread in the system. Thread data: 
[00:00:00.397,155] <inf> zperf_startup:       Name        : bt_tx_processor
[00:00:00.397,186] <inf> zperf_startup:       Priority    : -1
[00:00:00.397,186] <inf> zperf_startup:       Stack size  : 1024
[00:00:00.397,186] <inf> zperf_startup:       Custom data : 3
[00:00:00.397,216] <inf> zperf_startup: Iterating 4 thread in the system. Thread data: 
[00:00:00.397,247] <inf> zperf_startup:       Name        : BT LW WQ
[00:00:00.397,247] <inf> zperf_startup:       Priority    : 10
[00:00:00.397,247] <inf> zperf_startup:       Stack size  : 1408
[00:00:00.397,277] <inf> zperf_startup:       Custom data : 4
[00:00:00.397,277] <inf> zperf_startup: Iterating 5 thread in the system. Thread data: 
[00:00:00.397,308] <inf> zperf_startup:       Name        : sysworkq
[00:00:00.397,308] <inf> zperf_startup:       Priority    : -1
[00:00:00.397,308] <inf> zperf_startup:       Stack size  : 1024
[00:00:00.397,338] <inf> zperf_startup:       Custom data : 5
[00:00:00.397,338] <inf> zperf_startup: Iterating 6 thread in the system. Thread data: 
[00:00:00.397,369] <inf> zperf_startup:       Name        : logging
[00:00:00.397,369] <inf> zperf_startup:       Priority    : 14
[00:00:00.397,399] <inf> zperf_startup:       Stack size  : 768
[00:00:00.397,399] <inf> zperf_startup:       Custom data : 6
[00:00:00.397,399] <inf> zperf_startup: Iterating 7 thread in the system. Thread data: 
[00:00:00.397,430] <inf> zperf_startup:       Name        : idle
[00:00:00.397,460] <inf> zperf_startup:       Priority    : 15
[00:00:00.397,460] <inf> zperf_startup:       Stack size  : 320
[00:00:00.397,460] <inf> zperf_startup:       Custom data : 7
[00:00:00.397,491] <inf> zperf_startup: Iterating 8 thread in the system. Thread data: 
[00:00:00.397,521] <inf> zperf_startup:       Name        : main
[00:00:00.397,521] <inf> zperf_startup:       Priority    : 0
[00:00:00.397,521] <inf> zperf_startup:       Stack size  : 1024
[00:00:00.397,521] <inf> zperf_startup:       Custom data : 8
```

| Field | Description |
|-------|-------------|
| `Name` | Thread name registered at creation (`CONFIG_THREAD_NAME=y` required) |
| `Priority` | Zephyr thread priority; negative values are cooperative, positive are preemptive |
| `Stack size` | Allocated stack space in bytes |
| `Custom data` | `zperf`-assigned iterator index used internally to track the thread throughout its lifetime |

We use `Custom data` for assigning a task-iterator to track the Task later.

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
| `Task Id` | Internal `zperf` iterator index assigned at startup (matches `Custom data` from startup log) |
| `Total runtime` | Accumulated CPU time the thread spent executing during this window, in microseconds |
| `Max runtime` | Longest single continuous execution slice observed within the window, in microseconds |
| `Total Switches` | Number of times the thread was scheduled in during the window |

The footer line `Calculations Time slice` shows the total window duration in microseconds, and the CPU load summary reports `CPU LOAD` (active task time), `IDLE` (idle thread time), and `KERNEL` (scheduling overhead) as percentages of that time slice.

The tool relies on Zephyr RTOS tracing hooks to collect runtime statistics with minimal overhead. The tool integrates with the scheduler and interrupt tracing infrastructure through the following user hooks:

* `sys_trace_thread_switched_in_user()` – invoked when a thread is scheduled in. `zperf` records the start timestamp of the task execution period.
* `sys_trace_thread_switched_out_user()` – invoked when a thread is scheduled out. `zperf` calculates the elapsed runtime, updates the task's accumulated execution time, maximum runtime slice, and context switch counters.
* `sys_trace_isr_enter_user()` – called on ISR entry. `zperf` records the start of interrupt execution and temporarily suspends task runtime accounting (**paused for now**).
* `sys_trace_isr_exit_user()` – called on ISR exit. `zperf` measures ISR execution time and restores task runtime accounting (**paused for now**).
* `sys_trace_idle_user()` – called when the system enters the idle thread. This allows `zperf` to accurately track idle time and calculate overall CPU utilization.

By leveraging these RTOS-specific hooks, `zperf` continuously monitors task execution time in microseconds, context-switch activity, interrupt processing time, CPU load, idle percentage, and kernel overhead, providing a detailed real-time performance profile of the system.


### Task history

In addition to the per-window statistics, `zperf` maintains a fixed-size circular buffer of raw scheduling events. Each entry captures the thread name, the type of scheduler event, and the hardware timestamp in microseconds at which that event occurred.

The buffer is filled continuously as the scheduler runs and is printed at the end of each sampling window (when `CONFIG_ZPERF_TRACE_HISTORY=y`) or on demand by calling `zperf_trace_history()`. Because the buffer is circular, only the most recent `CONFIG_ZPERF_HISTORY_SIZE` events are retained.

Each row represents one scheduler event:

| Column | Description |
|--------|-------------|
| `Task Name` | Thread name as registered with Zephyr (`CONFIG_THREAD_NAME=y` required) |
| `State` | Scheduler event type: `IN` — thread scheduled in, `OUR` — thread scheduled out, `ISR_IN` — ISR entered, `ISR_OUT` — ISR exited, `IDLE` — idle thread running |
| `Timestamp` | Hardware timestamp in microseconds (`k_uptime_ticks` converted to µs) |

```
[00:00:07.461,791] <inf> zperf_history: # ############### Collected History ############### #
[00:00:07.461,791] <inf> zperf_history: #                                                   #
[00:00:07.461,791] <inf> zperf_history: # Task Name           State    Timestamp            #
[00:00:07.461,853] <inf> zperf_history: # idle                 IDLE      7375122            #
[00:00:07.461,883] <inf> zperf_history: # idle                 IDLE      7375336            #
[00:00:07.461,914] <inf> zperf_history: # idle                 IDLE      7375397            #
[00:00:07.461,944] <inf> zperf_history: # idle                 IDLE      7375488            #
[00:00:07.462,005] <inf> zperf_history: # idle                 IDLE      7375610            #
[00:00:07.462,036] <inf> zperf_history: # idle                 IDLE      7375763            #
[00:00:07.462,066] <inf> zperf_history: # idle                 IDLE      7375824            #
[00:00:07.462,097] <inf> zperf_history: # idle                 IDLE      7376892            #
[00:00:07.462,158] <inf> zperf_history: # idle                 IDLE      7377106            #
[00:00:07.462,188] <inf> zperf_history: # idle                 IDLE      7377380            #
[00:00:07.462,219] <inf> zperf_history: # idle                 IDLE      7377594            #
[00:00:07.462,249] <inf> zperf_history: # idle                 IDLE      7377869            #
[00:00:07.462,310] <inf> zperf_history: # idle                 IDLE      7378113            #
[00:00:07.462,341] <inf> zperf_history: # idle                  OUR      7461334            #
[00:00:07.462,371] <inf> zperf_history: # Task_ZPerf             IN      7461365            #
[00:00:07.462,402] <inf> zperf_history: # idle                  OUR      7338837            #
[00:00:07.462,463] <inf> zperf_history: # Task_BLE               IN      7338867            #
[00:00:07.462,493] <inf> zperf_history: # Task_BLE              OUR      7338898            #
[00:00:07.462,524] <inf> zperf_history: # idle                   IN      7338898            #
[00:00:07.462,554] <inf> zperf_history: # idle                 IDLE      7338928            #
[00:00:07.462,615] <inf> zperf_history: # idle                  OUR      7340637            #
[00:00:07.462,646] <inf> zperf_history: # Task_LED               IN      7340668            #
[00:00:07.462,677] <inf> zperf_history: # Task_LED              OUR      7340698            #
[00:00:07.462,707] <inf> zperf_history: # idle                   IN      7340698            #
[00:00:07.462,768] <inf> zperf_history: # idle                 IDLE      7340698            #
[00:00:07.462,768] <inf> zperf_history: # ################################################# #
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
docker run --rm -it \
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

[00:01:57.335,083] <inf> zperf: # #####################   1 Second System Performance   ###################### #
[00:01:57.335,083] <inf> zperf: #                                                                              #
[00:01:57.335,113] <inf> zperf: #   Task Id         Total runtime         Max runtime        Total Switches    #
[00:01:57.335,144] <inf> zperf: #      0                    458                 458                 1          #
[00:01:57.335,174] <inf> zperf: #      1                     61                  61                 4          #
[00:01:57.335,205] <inf> zperf: #      2                    154                  61                 4          #
[00:01:57.335,266] <inf> zperf: #      3                      0                   0                 0          #
[00:01:57.335,296] <inf> zperf: #      4                      0                   0                 0          #
[00:01:57.335,327] <inf> zperf: #      5                      0                   0                 0          #
[00:01:57.335,357] <inf> zperf: #      6                 162598              144135                 3          #
[00:01:57.335,418] <inf> zperf: #      7                 836944              107727                27          #
[00:01:57.335,449] <inf> zperf: #      8                      0                   0                 0          #
[00:01:57.335,449] <inf> zperf: # ############################################################################ #
[00:01:57.335,479] <inf> zperf: # ### Calculations Time slice: 1000549

[00:01:57.335,479] <inf> zperf: # ################# CPU LOAD 16% ### IDLE 83% ### KERNEL 0% ################## #

[00:01:58.335,632] <inf> zperf: # #####################   1 Second System Performance   ###################### #
[00:01:58.335,632] <inf> zperf: #                                                                              #
[00:01:58.335,662] <inf> zperf: #   Task Id         Total runtime         Max runtime        Total Switches    #
[00:01:58.335,693] <inf> zperf: #      0                    489                 489                 1          #
[00:01:58.335,723] <inf> zperf: #      1                     61                  61                 4          #
[00:01:58.335,754] <inf> zperf: #      2                    152                  62                 4          #
[00:01:58.335,784] <inf> zperf: #      3                      0                   0                 0          #
[00:01:58.335,845] <inf> zperf: #      4                      0                   0                 0          #
[00:01:58.335,876] <inf> zperf: #      5                      0                   0                 0          #
[00:01:58.335,906] <inf> zperf: #      6                 162749              124359                 3          #
[00:01:58.335,968] <inf> zperf: #      7                 836792              104920                25          #
[00:01:58.335,998] <inf> zperf: #      8                      0                   0                 0          #
[00:01:58.335,998] <inf> zperf: # ############################################################################ #
[00:01:58.335,998] <inf> zperf: # ### Calculations Time slice: 1000550

[00:01:58.336,029] <inf> zperf: # ################# CPU LOAD 16% ### IDLE 83% ### KERNEL 0% ################## #

[rtio 15:56:06] Quit rtio
```

## Authors

Maintained by [nullsych](https://github.com/nullsych).
