/* AVR programmer */

#include "f_cpu.h"

#include <avr/io.h>
#include <util/delay.h>

#include "avrprog.h"
#include "message.h"
#include "misc.h"
#include "prog.h"
#include "serial.h"
#include "spi.h"

int main() {
  initEmClock();
  initUSART();
  initSPI();

  /* // ------------------------------------------------- */
#if 0
  {
    unsigned char c;
    do {
      receive(&c);
      transmit(c);
    } while (c != 'q');
  }
  transmit(0x0d);
  transmit(0x0a);

  while(1) {
    const unsigned char h[16] = "0123456789ABCDEF";
    const unsigned char q[4] = { 0xAC, 0x53, 0, 0};
    unsigned char i;
    unsigned char c;

    PORTSPI |= _BV(RESET);
    _delay_us(50);
    PORTSPI &= ~_BV(RESET);

    _delay_ms(25);
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
    _delay_ms(1000);
  }
#endif
  /* // ------------------------------------------------- */

  while (1) {
    readMessage();
    processMessage();
    sendMessage();
  }
}
