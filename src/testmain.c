/* AVR programmer */

#define F_CPU 1000000UL

#include <avr/io.h>
#include <util/delay.h>
// #include <avr/eeprom.h>

int main() {
//   uint8_t i;
//
//   for (i = 0; i < 128; i ++) {
//     eeprom_update_byte((uint8_t *)(uint16_t)i, i);
//   }

  DDRB |= 1;

  while (1) {
    PORTB ^= 1;
    _delay_ms(500);
  }
}
