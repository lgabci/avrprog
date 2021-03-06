/* AVR programmer misc */

#include "misc.h"
#include <util/delay.h>

/* Initalize emergecy clock (for MCU to program)              */
void emClockInit(void) {
/* emergency external clock signal
   TCCR1A COM1A1  COM1A0  COM1B1  COM1B0  FOC1A   FOC1B   WGM11   WGM10
   TCCR1B ICNC1   ICES1   -       WGM13   WGM12   CS12    CS11    CS10
   TCNT1L TCNT1L7 TCNT1L6 TCNT1L5 TCNT1L4 TCNT1L3 TCNT1L2 TCNT1L1 TCNT1L0
   TCNT1H TCNT1H7 TCNT1H6 TCNT1H5 TCNT1H4 TCNT1H3 TCNT1H2 TCNT1H1 TCNT1H0
   OCR1AL ORC1AL7 ORC1AL6 ORC1AL5 ORC1AL4 ORC1AL3 ORC1AL2 ORC1AL1 ORC1AL0
   OCR1AH ORC1AH7 ORC1AH6 ORC1AH5 ORC1AH4 ORC1AH3 ORC1AH2 ORC1AH1 ORC1AH0
   OCR1BL ORC1BL7 ORC1BL6 ORC1BL5 ORC1BL4 ORC1BL3 ORC1BL2 ORC1BL1 ORC1BL0
   OCR1BH ORC1BH7 ORC1BH6 ORC1BH5 ORC1BH4 ORC1BH3 ORC1BH2 ORC1BH1 ORC1BH0
   ICR1L  IRC1L7  IRC1L6  IRC1L5  IRC1L4  IRC1L3  IRC1L2  IRC1L1  IRC1L0
   ICR1H  IRC1H7  IRC1H6  IRC1H5  IRC1H4  IRC1H3  IRC1H2  IRC1H1  IRC1H0
   TIMSK  -       -       TICIE1  OCIE1A  OCIE1B  TOIE1   -       -
   TIFR   -       -       ICF1    OCF1A   OCF1B   TOV1    -       -        */

  DDRB |= _BV(DDB1);                  /* set output pin        */
  TCCR1A = _BV(COM1A1) |              /* non-inverting mode    */
    _BV(WGM11);                       /* fast PWM, TOP = ICR1  */
  TCCR1B = _BV(WGM13) | _BV(WGM12) |  /* fast PWM, TOP = ICR1  */
    _BV(CS10);                        /* prescaler = 1         */
  ICR1 = 7;                           /* TOP = 7, 0 - 7        */
  OCR1A = 3;                     /* set = 0 - 3, clear = 4 - 7 */
}

/* string to hexadecimal converter
   -v: value to convert
   - str: pointer to strig where to put hexa string            */
void hex(const uint8_t v, int8_t *str) {
  str[0] = v >> 4 < 10 ? '0' + (v >> 4) : 'A' - 10 + (v >> 4);
  str[1] = (v & 0x0f) < 10 ? '0' + (v & 0x0f) : 'A' - 10 + (v & 0x0f);
  str[2] = 0;
}

/* wait ms
   -w: milliseconds to wait                    */
void delayMs(uint8_t w) {
  while (w --) {
    _delay_ms(1);
  }
}

/* wait us
   -w: microseconds to wait                    */
void delayUs(uint8_t w) {
  while (w --) {
    _delay_us(1);
  }
}
