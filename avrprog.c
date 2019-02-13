#define F_CPU 8000000UL

#include <avr/io.h>
#include <util/delay.h>

int main() {
  DDRC = 1 << PC5;

  while (1) {
    PORTC ^= (1 << PC5);
    _delay_ms(1000);
  }
  return 0;
}
