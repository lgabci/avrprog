/* AVR programmer SPI */

#include "misc.h"
#include "spi.h"

/* Initialize software SPI interface */
void spiInit(void) {
/* PIN   SPI   I/O
   PD4   SCK   O
   PD5   MISO  I
   PD6   MOSI  O
   PD7   RESET O
                                                                           */
  #define DDRSPI DDRD
  #define PORTSPI PORTD
  #define PINSPI PIND

  #define SCK   PORTD4
  #define MISO  PORTD5
  #define MOSI  PORTD6
  #define RESET PORTD7
  DDRSPI = (DDRSPI & ~ _BV(MISO)) | _BV(SCK) | _BV(MOSI) | _BV(RESET);
  /* MOSI, MISO (pull up), RESET, SCK low */
  PORTSPI &= ~ (_BV(MOSI) | _BV(MISO) | _BV(RESET) | _BV(SCK));
}

/* Switch on/off reset line
   - r: 0 = MCU reset line on, other = off  */
void spiReset(uint8_t r) {
  if (r) {
    PORTSPI |= _BV(RESET);
  }
  else {
    PORTSPI &= ~ _BV(RESET);
  }
}

/* Switch on/off SPI clock line
   - r: 0 = SPI clock line on, other = off  */
void spiSck(uint8_t c) {
  if (c) {
    PORTSPI |= _BV(SCK);
  }
  else {
    PORTSPI &= ~ _BV(SCK);
  }
}

/* Transmit and receive 1 byte
   - transv: value to transmit
   - return value: received byte     */
uint8_t spiTransmit(uint8_t transv) {
  uint8_t i;
  uint8_t recv;

  recv = 0;
  for (i = _BV(7); i; i >>= 1) {
    if (transv & i) {
      PORTSPI |= _BV(MOSI);
    }
    else {
      PORTSPI &= ~_BV(MOSI);
    }
    delayUs(5);   ////

    PORTSPI |= _BV(SCK);
    delayUs(5);   ////

    PORTSPI &= ~_BV(SCK);
    recv = recv << 1 | (PINSPI >> MISO & 0x01);
  }
  return recv;
}
