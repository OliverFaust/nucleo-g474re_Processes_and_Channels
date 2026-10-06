# What changes in the book chapter text (Processes and Channels), 2.0.1 -> 3.0.0

1. **Listings:** none. `application.cpp` compiles unchanged with 3.0 (`Channel<unsigned int>`,
   `Run(InParallel(sender, receiver), ExecutionMode::StaticNetwork, NETWORK_PRIORITY)`; 3.0 requires the
   `ExecutionMode`, which the listing already names).
2. **Project setup:** two defines in both configurations, `CSP4CMSIS_MAX_SYSCALL_INTERRUPT_PRIORITY=5` and
   `CSP4CMSIS_DEVICE_HEADER="stm32g4xx.h"`; static allocation is the default and FreeRTOS is detected.
3. **Library version:** CSP4CMSIS 3.0.0 (stable 3.x API), unmodified in `lib/csp4cmsis/`.
4. **Unchanged:** output, stacks (measured identical), priorities, memory (0 FreeRTOS heap allocations),
   CubeMX settings.
