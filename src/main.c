/* AVR programmer */

#include "misc.h"
#include "main.h"
#include "message.h"
#include "prog.h"
#include "serial.h"
#include "spi.h"
#include "lcd.h"  /////

int main() {
  lcdInit();
  emClockInit();
  usartInit();
  spiInit();

  /* // ------------------------------------------------- */
#if 0
  {
    uint8_t c;
    do {
      receive(&c);
      transmit(c);
    } while (c != 'q');
  }
  transmit(0x0d);
  transmit(0x0a);

  while(1) {
    const uint8_t h[16] = "0123456789ABCDEF";
    const uint8_t q[4] = { 0xAC, 0x53, 0, 0};
    uint8_t i;
    uint8_t c;

    PORTSPI |= _BV(RESET);
    delayUs(50);
    PORTSPI &= ~_BV(RESET);

    delayMs(25);
    for (i = 0; i < 4; i ++) {
      c = transmitSPI(q[i]);

      transmit(h[q[i] >> 4]);
      transmit(h[q[i] & 0x0f]);
      transmit('-');
      transmit(h[c >> 4]);
      transmit(h[c & 0x0f]);
      transmit(' ');
      transmit(' ');
    }
    transmit(' ');
    delayMs(1000);
  }
#endif
  /* // ------------------------------------------------- */

  while (1) {
    readMessage();
    processMessage();
    sendMessage();
  }
}
