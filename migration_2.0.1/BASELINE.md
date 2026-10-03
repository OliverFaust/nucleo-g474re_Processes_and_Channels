# Baseline before the CSP4CMSIS 2.0.1 update

**Date:** 2026-10-03. **Commit:** `6dc8ccd` (main, up to date with origin). **Library:** `lib/csp4cmsis/`,
the pre-1.0 FreeRTOS-native snapshot (tree `17c36e6`, the same as nucleo-g474re_The_Process before its
update). **Build:** STM32CubeIDE 2.1.0 headless, Debug (`-O0`, GNU++17); Release does not build (no
`lib/csp4cmsis/inc` include path and no GNU++17 in Release: 15 errors).
**Board:** NUCLEO-G474RE, ST-LINK-V3 VCP (LPUART1) 115200.

| | Debug |
|---|---|
| ELF SHA-256 | `68030496f8bd6da99de9b15d54ccdc7909e5d847d214af0c03d22e8b1044c1d0` |
| text / data / bss | 41 568 / 132 / 39 036 B |
| UART (20 s from reset) | welcome line, bootstrap banner, `--- Single Sender & Receiver with Infinite Loop ---`, then `Send: n Received: n` for n = 0, 1, 2, ...: 8235 complete lines (0..8234) in 20 s, none out of sequence (UART-bound, about 412 lines/s) |
| Sender / Receiver stack (`CSProcessStatic<256>`, 1024 B each) | 216 B / 532 B used |
| Priorities Sender / Receiver | 2 / 2 (native; MainApp 3, defaultTask 24, timer task 2, idle 0) |
| FreeRTOS heap (heap_4, 30 720 B) | 5 allocations, **0 frees**, 20 152 B free (10 568 B in use) |
| newlib `_sbrk` | 1032 B |

- **0 frees:** `MainApp` deletes itself (`vTaskDelete(NULL)`), but its 8 KB stack and TCB are freed
  by the idle task, which never runs: Sender and Receiver (priority 2) never block for long (the
  receiver prints continuously), so priority 0 never gets the CPU. The 8 KB stay allocated.
- Channel: `Channel<unsigned int>`, a blocking rendezvous (zero capacity), one writer, one reader; no
  policy, no ISR use. Same meaning in 2.0.1 (`Channel<T>` = `SamplingChannel<T, Block>`, rendezvous).
- Logs: `results/baseline_uart.txt`, `results/baseline_swd.txt`.
