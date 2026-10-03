# CSP4CMSIS Simple Sender‑Receiver Demo for NUCLEO-G474RE

A minimal demonstration of the **CSP (Communicating Sequential Processes)** library CSP4CMSIS using CMSIS‑RTOS v2 on an STM32G474RE microcontroller. This project implements a classic rendezvous channel between two processes: one sender that transmits an ever‑incrementing unsigned integer, and one receiver that prints each value. The formal CSP model is in [`Formal model/`](Formal%20model/).

## Features

- **FreeRTOS** with the CMSIS‑RTOS v2 API (STM32CubeMX `CMSIS_V2` interface)
- **CSP4CMSIS 2.0.1** library for channel‑based, deterministic concurrency
- **Rendezvous synchronisation** – sender and receiver meet exactly at each message exchange
- **No FreeRTOS heap allocation**: every thread's stack and control block is static (see [Memory](#memory))
- **Roll‑over counter** – `unsigned int` wraps from `UINT_MAX` to `0` automatically
- **Serial console output** via LPUART1, the ST‑LINK virtual COM port (115200 baud) – only the receiver prints, so no message interleaving

## Hardware Requirements

- STM32 Nucleo‑G474RE board  
- USB cable for power, programming, and serial communication  
- No external components required

## Software Requirements

Tested with:

| Tool | Version |
|---|---|
| STM32CubeIDE | 2.1.0 (GNU Tools for STM32 14.3.rel1) |
| STM32CubeMX (only to regenerate code) | 6.17.0 |
| STM32Cube FW_G4 | V1.6.3 (FreeRTOS 10.3.1) |
| CSP4CMSIS | 2.0.1, in `lib/csp4cmsis/` (unmodified; see `lib/csp4cmsis/VERSION`) |

## Serial Configuration

| Parameter   | Value          |
|-------------|----------------|
| Baud Rate   | 115200         |
| Data Bits   | 8              |
| Stop Bits   | 1              |
| Parity      | None           |
| Flow Control| None           |

## Building with STM32CubeIDE

1. **Clone this repository** (do not place it inside your STM32CubeIDE workspace directory).  
2. Open STM32CubeIDE.  
3. Go to `File → Import → Existing Projects into Workspace`.  
4. Select the cloned directory.  
5. Build the project (configuration `Debug` or `Release`).  
6. Flash the binary to your Nucleo board.

The CSP4CMSIS settings are already in the project (G++ compiler, Debug and Release): include path `../lib/csp4cmsis/inc`, and the defines `CSP4CMSIS_RTOS2_BACKEND_FREERTOS`, `CSP4CMSIS_MAX_SYSCALL_INTERRUPT_PRIORITY=5`, `CSP4CMSIS_STATIC_ALLOCATION` and `CSP4CMSIS_DEVICE_HEADER="stm32g4xx.h"` (explained in the [CSP4CMSIS STM32CubeIDE guide](https://github.com/OliverFaust/CSP4CMSIS/blob/main/Documentation/CSP4CMSIS_STM32CubeIDE.md)).

## Regenerating code with STM32CubeMX

`nucleo-g474re_v10.ioc` can be opened and regenerated (GENERATE CODE) without losing anything: the application's code in `main.c` and `FreeRTOSConfig.h` sits between `USER CODE BEGIN`/`END` markers, and the FreeRTOS settings it needs (heap size, newlib reentrancy, static default task) are stored in the `.ioc`.

## Project Structure
```text
├── Core/            # main.c (CubeMX), application.cpp (the example)
├── Drivers/         # STM32 HAL, CMSIS and BSP drivers
├── Formal model/    # CSP-M model of the sender-receiver network
├── lib/csp4cmsis/   # CSP4CMSIS 2.0.1 (inc/, src/, LICENSE, VERSION)
├── Middlewares/     # FreeRTOS + CMSIS‑RTOS v2
├── nucleo-g474re_v10.ioc  # STM32CubeMX project
└── README.md
```

## How It Works

1. **Channel**: `static Channel<MessageType> chan;` – a blocking rendezvous channel with zero capacity, for one writer and one reader. Elements are copied, so the element type must be trivially copyable (`unsigned int` is).  
2. **Sender process**: runs an infinite loop, sending the current value of `counter` through `out << counter`. The send operation blocks until the receiver has taken the value. After sending, `counter` increments (rolls over automatically).  
3. **Receiver process**: runs an infinite loop, waiting for a message with `in >> received`. The receive operation blocks until the sender has sent a value. Once received, it prints `Send: X Received: X`: only the receiver prints, and it prints the value it received twice – because of the rendezvous, that is exactly the value the sender sent.  
4. **Parallel composition**: `InParallel(sender, receiver)` composes both processes; `Run(..., ExecutionMode::StaticNetwork, priority)` creates their threads and returns, and the network runs for ever.  
5. **Start-up**: `main.c` calls `csp_app_main_init()`, which creates the `MainApp` thread (static 1 KB stack). `MainApp` prints the banner, starts the network and exits. `MainApp` runs at a higher priority (`osPriorityBelowNormal`) than the network (`osPriorityLow`), so `Sender` and `Receiver` first run after `MainApp` has exited.

Because the channel is a rendezvous (capacity 0), the two processes are perfectly synchronised – every value sent is immediately received and printed. The receiver prints continuously, so the output rate is set by the console: about 410 lines per second at 115200 baud.

## Example Console Output

```text
Welcome to STM32 world !

=== STM32 FreeRTOS + CSP4CMSIS bootstrap ===

--- Single Sender & Receiver with Infinite Loop ---
Send: 0 Received: 0
Send: 1 Received: 1
Send: 2 Received: 2
Send: 3 Received: 3
...
Send: 4294967295 Received: 4294967295
Send: 0 Received: 0
...
```

## Memory

Measured on the board (Debug and Release, after 20 s):

- **FreeRTOS heap: not used.** `pvPortMalloc()` is never called (0 allocations). `Sender`, `Receiver`, `MainApp`, CubeMX's `defaultTask`, and FreeRTOS's idle and timer tasks all have static stacks and control blocks; the channel needs no RTOS objects of its own. The FreeRTOS heap (`configTOTAL_HEAP_SIZE`) is therefore set to only 1 KB: enough for one small dynamically created thread (a 128‑word stack and its control block) if you switch one back to dynamic allocation.
- **C library heap: 1 KB.** newlib's `printf()` allocates its `stdout` buffer with `malloc()` on first use (1032 B from `_sbrk()`). This is the only dynamic allocation.
- **Stacks used** (Debug; Release in brackets): `Sender` 320 B (212 B) of 1 KB, `Receiver` 532 B (492 B) of 1 KB, `MainApp` 500 B (300 B) of 1 KB, `defaultTask` 128 B (96 B) of 2 KB.

## Key CSP4CMSIS Concepts Demonstrated

- **Process** – a class derived from `CSProcessStatic<N>` (static N‑word stack) with a `run()` function.
- **Chanout / Chanin** – typed channel ends for sending (`<<`) and receiving (`>>`).
- **Channel** – synchronous rendezvous between processes.
- **Static network** – processes created once at start-up, with static stacks and control blocks, running for ever.
- **Process composition** – `InParallel` combines independent processes.
- **External choice** (not used here, but supported via `Alternative`).

## Troubleshooting

- **No output on serial**: Verify the baud rate and that the correct COM port (the ST‑LINK virtual COM port) is used.
- **Program hangs**: The rendezvous channel blocks forever if only one process runs. Ensure both `Sender` and `Receiver` are included in `InParallel`.
- **`configASSERT failed: <file>:<line>`** on the console: a FreeRTOS assertion failed at that source line; the program halts there.

## License

MIT License – see the `LICENSE` file. CSP4CMSIS: MIT License, `lib/csp4cmsis/LICENSE`.

## Acknowledgments

- STMicroelectronics for the STM32 HAL and CMSIS‑RTOS v2
- The FreeRTOS team
- [CSP4CMSIS](https://oliverfaust.github.io/CSP4CMSIS/) library by Oliver Faust
