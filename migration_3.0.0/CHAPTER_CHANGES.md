# What changes in the book chapter text (Processes and Channels), 2.0.1 -> 3.0.0

1. **Listings:** none. `application.cpp` compiles unchanged with 3.0 (`Channel<unsigned int>`,
   `Run(InParallel(sender, receiver), ExecutionMode::StaticNetwork, NETWORK_PRIORITY)`; 3.0 requires the
   `ExecutionMode`, which the listing already names).
2. **Project setup:** two defines in both configurations, `CSP4CMSIS_MAX_SYSCALL_INTERRUPT_PRIORITY=5` and
   `CSP4CMSIS_DEVICE_HEADER="stm32g4xx.h"`; static allocation is the default and FreeRTOS is detected.
3. **Library version:** CSP4CMSIS 3.0.0 (stable 3.x API), unmodified in `lib/csp4cmsis/`.
4. **Unchanged:** output, stacks (measured identical), priorities, memory (0 FreeRTOS heap allocations),
   CubeMX settings.
5. **No heap at all (branch `unbuffered-stdout`):** `main.c` (USER CODE 2) makes `stdout` unbuffered with
   `setvbuf(stdout, NULL, _IONBF, 0)`, so newlib's `printf()` no longer allocates its 1 KB `stdout` buffer:
   `_sbrk()` is never called (measured), as in Alternation and the Sensor chapter. The start-up banner
   becomes `--- Single Sender & Receiver with Infinite Loop (Zero-Heap) ---` (as in those two chapters), and the text can say that the program
   allocates no heap memory at all (FreeRTOS heap: 0 allocations; C library heap: not used).
6. **Simplified listing (branch `simplify-chapter-code`):** the code shows the chapter's concept and
   nothing else.
   - `using MessageType = unsigned int;` is gone: the channel and the ends are `Channel<unsigned int>`,
     `Chanout<unsigned int>`, `Chanin<unsigned int>`.
   - `Sender` and `Receiver` lose their `name()` overrides; the constructors are `explicit`; the Sender
     writes `out << counter++;`.
   - **`main.c`:** CubeMX's `defaultTask` is removed (its attributes, its creation, `StartDefaultTask`).
     The program's threads are now exactly the ones in `application.cpp` (MainApp and the processes),
     plus FreeRTOS's idle and timer tasks. The `.ioc` still contains the task: CubeMX does not allow a
     project without one and re-creates it on regeneration (from the `.ioc`, as the static, heap-free
     task it was); the README says to delete it again.
   - **Thread names:** without the `name()` overrides, the processes appear as `csp_task` in a
     debugger's thread view; MainApp keeps its name (`attr.name`).
   - **Comments** shortened to what is surprising; the explanations are in the chapter text.
   - The console output is unchanged.
