/* AVR programmer */

#define F_CPU 7372800UL

#include <avr/io.h>
#include <util/delay.h>

#include "avrprog.h"
#include "command.h"

#define MAXMSGSIZE 100
unsigned char seq;              /* message sequence    */
unsigned short int msgsize;     /* message size in msg */
unsigned char msg[MAXMSGSIZE];  /* message body        */

/* PARAM_RESET_POLARITY
     0 = active high reset (AT89)
     1 = active low reset (AVR) */
unsigned char paramResetPolarity = 1;
/* PARAM_CONTROLLER_INIT
     set 0 at reset, host can set it, and test if the power has been lost */
unsigned char paramControllerInit = 0;

unsigned char statusReg = STATUS_CMD_OK;

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
