/* AVR programmer */

#include "misc.h"
#include "main.h"
#include "message.h"
#include "prog.h"
#include "serial.h"
#include "spi.h"
#include "led.h"

int main() {
  ledInit();
  emClockInit();
  usartInit();

  while (1) {
    readMessage();
    processMessage();
    sendMessage();
  }
}
