#define F_CPU 8000000UL

#include <avr/io.h>
#include <util/delay.h>

#define BAUD 9600
int main() {

  UBRRH = (F_CPU / 16 / BAUD - 1) >> 8;
  UBRRL = F_CPU / 16 / BAUD - 1;
  UCSRB = (1 << RXEN) | (1 << TXEN);
  UCSRC = (1 << URSEL) | (1 << USBS) | (3 << UCSZ0);

  while (1) {
    unsigned char c;

    while ( ! (UCSRA & (1 << RXC)))
      ;
    c = UDR;

    while ( ! (UCSRA & (1 << UDRE)))
      ;
    UDR = c;
  }
  return 0;
}
