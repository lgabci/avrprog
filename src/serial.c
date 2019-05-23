/* AVR programmer serial */

#include "f_cpu.h"

#include <avr/io.h>

#include "serial.h"

void initUSART(void) {
/* The STK500 uses: 115.2 kbps, 8 data bits, 1 stop bit, no parity
   UDR    ----------------------- UART data --------------------------
   UCSRA  RXC     TXC     UDRE    FE      DOR     PE      U2X     MPCM
   UCSRB  RXCIE   TXCIE   UDRIE   RXEN    TXEN    UCSZ2   RXB8    TXB8
   UCSRC  URSEL   UMSEL   UPM1    UPM0    USBS    UCSZ1   UCSZ0   UCPOL
   UBRRH  URSEL   -       -       -       UBRR3   UBRR2   UBRR1   UBRR0
   UBRRL  UBBR7   UBBR6   UBBR5   UBBR4   UBBR3   UBBR2   UBBR1   UBBR0   */

#define BAUD 115200
  UBRRH = (F_CPU / 16 / BAUD - 1) >> 8;
  UBRRL = F_CPU / 16 / BAUD - 1;
#undef BAUD
  UCSRA = 0;
  UCSRC = _BV(URSEL) | _BV(UCSZ1) | _BV(UCSZ0);
  UCSRB = _BV(RXEN) | _BV(TXEN);
}

/* Receive a character from USART, returns error */
unsigned char receive(unsigned char *c) {
  unsigned char err;

  while (! (UCSRA & _BV(RXC)))
    ;
  err = UCSRA & (_BV(FE) | _BV(DOR));
  *c = UDR;
  return ! err;
}

/* Send a character on USART */
void transmit(unsigned char c) {
  while (! (UCSRA & _BV(UDRE)));
  UDR = c;
}
