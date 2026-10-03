#include <cstdio>

#include "cmsis_os2.h"
#include "FreeRTOS.h"  // StaticTask_t: the control block of a statically created thread
#include "csp/csp4cmsis.h"

using namespace csp;

using MessageType = unsigned int;

static Channel<MessageType> chan;

class Sender : public CSProcessStatic<256> {
 private:
  Chanout<MessageType> out;

 public:
  Sender(Chanout<MessageType> w) : out(w) {}
  const char* name() const override { return "Sender"; }

  void run() override {
    unsigned int counter = 0;
    while (true) {
      out << counter;
      counter++;
    }
  }
};

class Receiver : public CSProcessStatic<256> {
 private:
  Chanin<MessageType> in;

 public:
  Receiver(Chanin<MessageType> r) : in(r) {}
  const char* name() const override { return "Receiver"; }

  void run() override {
    MessageType received;
    while (true) {
      in >> received;
      printf("Send: %u Received: %u\r\n", received, received);
    }
  }
};

// Start order. MainApp runs at a higher priority than the network it launches, so
// Run(..., StaticNetwork) only creates the Sender and Receiver threads and returns: neither
// can preempt MainApp, and they first run after MainApp has printed its banner and exited.
// Both stay below CubeMX's defaultTask (osPriorityNormal), as before.
static constexpr osPriority_t MAIN_APP_PRIORITY = osPriorityBelowNormal;
static constexpr osPriority_t NETWORK_PRIORITY  = osPriorityLow;

// MainApp's stack and control block are static: creating the thread takes no heap.
// CMSIS-RTOS2 counts the stack in bytes: 256 words = 1 KB. Measured on the NUCLEO-G474RE:
// MainApp uses 500 B (Debug, -O0) and 300 B (Release, -Os) of it.
alignas(8) static uint32_t mainAppStack[256];
static StaticTask_t mainAppControlBlock;

void MainApp_Task(void* argument) {
  (void)argument;
  osDelay(10);
  printf("\r\n--- Single Sender & Receiver with Infinite Loop ---\r\n");

  static Sender sender(chan.writer());
  static Receiver receiver(chan.reader());

  Run(InParallel(sender, receiver), ExecutionMode::StaticNetwork, NETWORK_PRIORITY);

  osThreadExit();
}

void csp_app_main_init(void) {
  osThreadAttr_t attr = {};
  attr.name       = "MainApp";
  attr.stack_mem  = mainAppStack;
  attr.stack_size = sizeof(mainAppStack);
  attr.cb_mem     = &mainAppControlBlock;
  attr.cb_size    = sizeof(mainAppControlBlock);
  attr.priority   = MAIN_APP_PRIORITY;
  if (osThreadNew(MainApp_Task, NULL, &attr) == NULL) {
    printf("ERROR: MainApp_Task creation failed!\r\n");
  }
}
