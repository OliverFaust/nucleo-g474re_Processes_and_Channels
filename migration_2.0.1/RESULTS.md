# CSP4CMSIS 2.0.1 update: results

**Date:** 2026-10-04. **Board:** NUCLEO-G474RE (ST-LINK-V3, VCP = LPUART1, 115200). **Tools:** STM32CubeIDE
2.1.0 (GNU Tools for STM32 14.3.1, headless build), STM32CubeMX 6.17.0, FW_G4 V1.6.3 (FreeRTOS 10.3.1),
STM32CubeProgrammer (flash; SWD reads in hotplug mode, no reset). **Library:** CSP4CMSIS v2.0.1,
unmodified. Procedure: `CHECKLIST.md` of nucleo-g474re_The_Process. Baseline: `BASELINE.md`. Scripts:
`run.sh` (this example's stacks and TCBs), `measure.py`, `seqcheck.py`.

## Analysis: nothing changes meaning

| Item | Finding |
|---|---|
| Channels | one: `static Channel<unsigned int> chan`, one writer (`Sender`), one reader (`Receiver`) |
| Type, policy | `Channel<T>` = `SamplingChannel<T, BufferPolicy::Block>`, a blocking zero-capacity rendezvous, in the old snapshot **and** in 2.0.1; no explicit policy |
| Element type | `unsigned int`: trivially copyable (2.0.1 checks this at compile time) |
| Rendezvous vs buffered | rendezvous; old: FreeRTOS mutex + task notifications, both sides block until the transfer; 2.0.1: critical section + thread flags, same blocking behaviour, no RTOS objects |
| ISR use, ALT, timeouts | none |
| Library | same pre-1.0 snapshot as nucleo-g474re_The_Process (tree `17c36e6`) |

## Commits (branch csp4cmsis-2.0.1, from main `6dc8ccd`, which was up to date with origin)

| Commit | Change |
|---|---|
| Baseline | `BASELINE.md`, logs |
| Prepare for regeneration | heap size (30 720, unchanged) and `USE_NEWLIB_REENTRANT` in the `.ioc`; bootstrap lines into `USER CODE BSP` |
| Migrate to FW_G4 V1.6.3 | 34 HAL/CMSIS/BSP files; `Middlewares/` (FreeRTOS 10.3.1) unchanged |
| CubeMX: static defaultTask; configASSERT | `defaultTask` static; printing `configASSERT` |
| CSP4CMSIS 2.0.1 | library + LICENSE + VERSION; include path and defines in Debug and Release (+ GNU++17 in Release); `application.cpp` on CMSIS-RTOS2; MainApp stack 1 KB and FreeRTOS heap 1 KB, sized from the measurements |
| Untrack language.settings.xml | gitignored; CubeIDE recreates it on import |
| LICENSE | "Copyright (c) 2026 Oliver Faust" (was "2024 Your Name"); first commit 2026-03-30 |
| README | versions, measured memory, corrections |

## Board results

| | Baseline (old library, Debug) | 2.0.1 Debug | 2.0.1 Release |
|---|---|---|---|
| Build | Debug only (Release: 15 errors) | 0 errors, 0 warnings | 0 errors, 0 warnings |
| text / data / bss (B) | 41 568 / 132 / 39 036 | 43 372 / 132 / 13 264 | 26 532 / 112 / 13 176 |
| UART header (from reset) | Welcome, bootstrap banner, `--- Single Sender & Receiver with Infinite Loop ---` | **identical** | **identical** |
| UART lines (20 s) | `Send: n Received: n`, n = 0..8234, none out of sequence | n = 0..8236, none out of sequence | n = 0..8357, none out of sequence |
| Sender / Receiver stack (1 KB each) | 216 / 532 B | 320 / 532 B | 212 / 492 B |
| MainApp stack | 8 KB from the heap (not measured) | 500 B of 1 KB (static) | 300 B of 1 KB |
| defaultTask stack (2 KB) | heap (not measured) | 128 B (static) | 96 B |
| Priorities Sender, Receiver / MainApp / defaultTask | 2, 2 / 3 / 24 | 8, 8 / 16 / 24 (from the TCBs) | 8, 8 / 16 / 24 |
| FreeRTOS heap | 5 allocations, 0 frees (10 568 B in use) | **0 allocations** (heap 1 KB) | **0 allocations** |
| newlib `_sbrk` | 1032 B | 1032 B | 1032 B |

- **Output:** same header and the same sequence; the line order cannot differ (only the receiver
  prints, and the rendezvous delivers the values in order). The number of lines in 20 s differs by a
  few because the log window starts at a slightly different time after reset; the rate is set by the
  UART (about 412 lines/s).
- **Stacks:** Sender's deepest use grows from 216 to 320 B in Debug (`-O0`; 212 B in Release);
  Receiver's (532 B, its `printf` path) is unchanged in Debug. Both keep at least 492 B unused.
- **Baseline heap, 0 frees:** MainApp deleted itself, but FreeRTOS frees a deleted task's memory in the
  idle task, which never ran: the network (priority 2, above idle) never blocks for long, because the
  receiver prints continuously. The idle task still never runs in 2.0.1, which no longer matters:
  nothing is freed, all memory is static.
- **Timer task:** the old network ran at native priority 2, the same as the FreeRTOS timer task
  (time-sliced); now it runs above it. Nothing uses software timers (2.0.1 needs none).
- **Start order:** MainApp (16) above the network (8), as before (3 above 2); checked by the priority
  readings and the output, not instrumented (The_Process's instrumented check covers the same
  bootstrap).

## Regeneration and fresh clone

- **GENERATE CODE** (CubeMX 6.17.0) on the committed `.ioc`: no change in git; rebuilt Debug and
  Release ELFs byte-identical to those flashed above, so the board output is identical.
- **Fresh clone** to another path, empty workspace, import, build (Debug and Release, 0 errors, 0
  warnings), flash: Release ELF byte-identical; Debug flash image identical (the ELF's debug
  information contains the build path); board output and measurements identical
  (`results/fresh_debug_*`). `language.settings.xml` was recreated by the import (ignored by git).

## What changes in the book chapter text (Processes and Channels)

**Channels** (the subject of the chapter):

1. **Declaration and use are unchanged.** `static Channel<MessageType> chan;`, `chan.writer()`,
   `chan.reader()`, `Chanout<T>`/`Chanin<T>`, `out << x`, `in >> x` compile and behave as before:
   - `Channel<T>` is still a blocking, zero-capacity rendezvous;
   - both sides block until the value has been transferred;
   - values arrive in order, none is lost or duplicated (measured: 8 237 values in sequence).
2. **What a channel is made of has changed.** Text that explains the implementation (a FreeRTOS mutex
   and task notifications) must change: in 2.0.1, a rendezvous channel uses a short critical section
   (BASEPRI) and CMSIS-RTOS2 thread flags. It creates **no RTOS object**: no mutex, queue or
   semaphore, and therefore no heap.
3. **New, explicit rules for rendezvous channels** (checked at compile time):
   - the element type must be **trivially copyable**, because elements are copied with `memcpy`;
   - only the **Block** policy is allowed: KeepNewest/KeepOldest need a buffer
     (`SamplingBufferedChannel<T, 1, P>`);
   - there is **no interrupt write path**: interrupts write only to buffered channels, via
     `isrWriter()` (relevant to the Interrupts chapter, worth a forward reference here).
4. **Channel names:**
   - `Channel<T>` is an alias for `SamplingChannel<T, BufferPolicy::Block>`;
   - `One2OneChannel` and `Any2OneChannel` are aliases of the same type;
   - `BufferedChannel<T, N>` is the buffered counterpart.

   If the text names the classes, use these names.
5. **The printed line:**
   - `Send: X Received: X` is printed by the receiver alone, showing the received value twice;
   - the "Send" figure equals the sent value only because of the rendezvous;
   - describe it that way (or change the code, which would change the output).

**Processes and start-up:**

6. **`application.cpp` listing:**
   - `osThreadNew` with an attribute structure: static 1 KB stack and control block, with the
     measured use in a comment;
   - `osDelay(10)`, `osThreadExit()`;
   - includes `cmsis_os2.h` and `FreeRTOS.h` (for `StaticTask_t`);
   - two named priority constants with the start-order comment; `Run()` takes the priority as a
     third argument.
7. **Stack units:** `CSProcessStatic<256>` counts words (1 KB), while `osThreadAttr_t.stack_size`
   counts bytes. MainApp's stack is 256 words = 1 KB, sized from the measured 500 B; the old
   `xTaskCreate(..., 2048, ...)` meant 2048 words = 8 KB.
8. **Priorities:**
   - Sender and Receiver run at `osPriorityLow` (8), MainApp at `osPriorityBelowNormal` (16) and
     defaultTask at `osPriorityNormal` (24); formerly 2, 3 and 24;
   - `Run()`'s default composition priority is now `osPriorityLow`;
   - the network never blocks for long (the receiver prints continuously), so the idle task never
     runs. This was already true before, and it is why the old dynamic MainApp was never freed.
9. **Start-up:** `csp_app_main_init()` is called in `USER CODE BSP` of `main.c`; the `USER CODE`
   markers are needed for CubeMX regeneration.
10. **CubeMX settings:**
    - `USE_NEWLIB_REENTRANT` Enabled;
    - heap size 1024 B in the `.ioc`;
    - `defaultTask` Allocation Static;
    - the printing `configASSERT`;
    - FW_G4 V1.6.3, CubeMX 6.17.0, CubeIDE 2.1.0.
11. **Project setup:**
    - `lib/csp4cmsis/` holds CSP4CMSIS 2.0.1, unmodified, with `VERSION` and `LICENSE`;
    - include path `../lib/csp4cmsis/inc`;
    - the four defines and GNU++17 in both configurations;
    - CSP4CMSIS is a CMSIS-RTOS2 library.
12. **Memory:**
    - not "zero heap" but: no FreeRTOS heap allocation (measured: 0; heap 1 KB), plus a 1 KB
      `stdout` buffer from newlib's `printf`;
    - stack figures as measured (Sender 320 B, Receiver 532 B of 1 KB, Debug);
    - "all channels and processes in `.data`/`.bss`" is now literally true.
13. **Console:** LPUART1 via the ST-LINK virtual COM port, not USART1.
14. **Unchanged:**
    - the console output;
    - the Sender and Receiver classes;
    - the channel declaration;
    - `InParallel` and `ExecutionMode::StaticNetwork`;
    - the formal model (`Formal model/`).

Logs: `results/` (UART logs with ELF SHA-256; `*_swd.txt`: SWD readings).
