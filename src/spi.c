/* AVR programmer SPI */

#include "f_cpu.h"

#include <avr/io.h>
#include <util/delay.h>

#include "spi.h"

void initSPI(void) {
/* PIN   SPI   I/O
   PC5   SCK   O
   PC4   MISO  I
   PC3   MOSI  O
   PC2   RESET O
                                                                           */
  #define DDRSPI DDRC
  #define PORTSPI PORTC
  #define PINSPI PINC

  #define SCK 5
  #define MISO 4
  #define MOSI 3
  #define RESET 2
  DDRSPI = (DDRSPI & ~_BV(MISO)) | _BV(SCK) | _BV(MOSI) | _BV(RESET);
  /* MOSI, MISO (pull up), RESET, SCK low */
  PORTSPI = PORTSPI & ~(_BV(MOSI) | _BV(MISO) | _BV(RESET) | _BV(SCK));
}

unsigned char transmitSPI(unsigned char transv) {
  unsigned char i;
  unsigned char recv;

  recv = 0;
  for (i = _BV(7); i; i >>= 1) {
    if (transv & i) {
      PORTSPI |= _BV(MOSI);
    }
    else {
      PORTSPI &= ~_BV(MOSI);
    }
    _delay_us(5);

    PORTSPI |= _BV(SCK);
    _delay_us(5);

    PORTSPI &= ~_BV(SCK);
    recv = recv << 1 | (PINSPI >> MISO & 0x01);
  }
  return recv;
}
