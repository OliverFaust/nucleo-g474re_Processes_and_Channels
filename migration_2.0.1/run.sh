#!/bin/bash
# run.sh <elf> <tag>: board run (measure.py), SWD readings and sequence check (seqcheck.py) for this
# example. Logs go to $OUT (default: .). Needs NUCLEO_RUN and STM32_PROGRAMMER_CLI (see measure.py).
S=${OUT:-.}; M=$(dirname $0); E=$1; T=$2
python3 $M/measure.py $E $S/${T}_uart.txt 20 \
  --stack 'Sender=MainApp_Task(void*)::sender+0x10:1024' --stack 'Receiver=MainApp_Task(void*)::receiver+0x10:1024' \
  --stack 'MainApp=mainAppStack' --stack 'defaultTask=defaultTaskBuffer' \
  --tcb 'Sender=MainApp_Task(void*)::sender+0x410' --tcb 'Receiver=MainApp_Task(void*)::receiver+0x410' \
  --tcb 'MainApp=mainAppControlBlock' --tcb 'defaultTask=defaultTaskControlBlock' > $S/${T}_measure.txt
cat $S/${T}_measure.txt; python3 $M/seqcheck.py $S/${T}_uart.txt
