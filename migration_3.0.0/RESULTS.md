# CSP4CMSIS 3.0.0 update: results

**Date:** 2026-10-06. **Board, tools:** as for 2.0.1 (`../migration_2.0.1/RESULTS.md`): NUCLEO-G474RE,
STM32CubeIDE 2.1.0 (GNU Tools for STM32 14.3.1), STM32CubeMX 6.17.0, FW_G4 V1.6.3. **Library:** CSP4CMSIS
v3.0.0 (commit `647a1cb`), unmodified. Procedure: `CHECKLIST.md` section 9 of nucleo-g474re_The_Process.
Scripts: `../migration_2.0.1/run.sh`, `measure.py`, `seqcheck.py` (unchanged; the process object layout is
the same in 3.0). Reference: the 2.0.1 logs `../migration_2.0.1/results/final_*`.

## Commits (branch csp4cmsis-3.0.0, from main `379ac23`)

| Commit | Change |
|---|---|
| lib/csp4cmsis | unmodified v3.0.0 sources, `LICENSE`, `VERSION` |
| Debug and Release defines | only `CSP4CMSIS_MAX_SYSCALL_INTERRUPT_PRIORITY=5` and `CSP4CMSIS_DEVICE_HEADER="stm32g4xx.h"` |
| README | 3.0.0, the two defines |

No application change: `application.cpp` uses no name that 3.0 changed (`Channel<unsigned int>`, `<<`/`>>`,
`Run(InParallel(...), ExecutionMode::StaticNetwork, NETWORK_PRIORITY)`).

## Board results

| | 2.0.1 Debug | 3.0.0 Debug | 2.0.1 Release | 3.0.0 Release |
|---|---|---|---|---|
| Build | 0 errors, 0 warnings | 0 errors, 0 warnings | 0 errors, 0 warnings | 0 errors, 0 warnings |
| text / data / bss (B) | 43 372 / 132 / 13 264 | 43 224 / 132 / 13 264 | 26 532 / 112 / 13 176 | 26 428 / 112 / 13 176 |
| UART header (from reset) | Welcome, banner, `--- Single Sender & Receiver with Infinite Loop ---` | **identical** | same | **identical** |
| `Send: n Received: n` (20 s) | 0..8236 in sequence | 0..8306 in sequence | 0..8357 in sequence | 0..8360 in sequence (see below) |
| Stacks used: Sender / Receiver / MainApp / defaultTask | 320 / 532 / 500 / 128 B | **identical** | 212 / 492 / 300 / 96 B | **identical** |
| Priorities | 8 / 8 / 16 / 24 | identical | 8 / 8 / 16 / 24 | identical |
| FreeRTOS heap / newlib `_sbrk` | 0 allocations / 1032 B | identical | 0 allocations / 1032 B | identical |

- **Release, "out-of-sequence: 1":** the log's last line is `Send: 8360 Received: 8`, cut off by the end of
  the 20 s capture; every complete line is in sequence. The number of lines in 20 s depends on where the
  capture starts and ends (continuous output; compare the sequence, not the count).
- **Static allocation without the define:** 0 FreeRTOS heap allocations (FreeRTOS detected from
  `FreeRTOS.h`).

## Regeneration and fresh clone

- **GENERATE CODE** on the committed `.ioc`: no change in git; rebuilt ELFs byte-identical (Debug
  `c65c0aa3…`, Release `e9707d47…`, as on the board).
- **Fresh clone** to another path, empty workspace, import, build (0 errors, 0 warnings both): Release
  ELF byte-identical, Debug flash image identical; flashed: header identical, values 0..8258 in sequence
  (`results/fresh_debug_uart.txt`).
