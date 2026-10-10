#include <cstdio>

#include "cmsis_os2.h"
#include "FreeRTOS.h"  // StaticTask_t
#include "csp/csp4cmsis.h"

using namespace csp;

static Channel<unsigned int> chan;

class Sender : public CSProcessStatic<256> {
  Chanout<unsigned int> out;

 public:
  explicit Sender(Chanout<unsigned int> w) : out(w) {}

  void run() override {
    unsigned int counter = 0;
    while (true) {
      out << counter++;
    }
  }
};

class Receiver : public CSProcessStatic<256> {
  Chanin<unsigned int> in;

 public:
  explicit Receiver(Chanin<unsigned int> r) : in(r) {}

  void run() override {
    unsigned int received;
    while (true) {
      in >> received;
      printf("Send: %u Received: %u\r\n", received, received);
    }
  }
};

// MainApp runs above the network, so Sender and Receiver first run after MainApp has
// printed its banner and exited.
static constexpr osPriority_t MAIN_APP_PRIORITY = osPriorityBelowNormal;
static constexpr osPriority_t NETWORK_PRIORITY  = osPriorityLow;

// Static stack (256 words = 1 KB) and control block: no heap.
alignas(8) static uint32_t mainAppStack[256];
static StaticTask_t mainAppControlBlock;

void MainApp_Task(void* argument) {
  (void)argument;
  osDelay(10);
  printf("\r\n--- Single Sender & Receiver with Infinite Loop (Zero-Heap) ---\r\n");

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
