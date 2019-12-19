/* AVR programmer SPI */

#include "misc.h"
#include "spi.h"
#include <util/delay_basic.h>

#define DDRSPI DDRD
#define PORTSPI PORTD
#define PINSPI PIND

#define SCK   PORTD4
#define MISO  PORTD5
#define MOSI  PORTD6
#define RESET PORTD7

static int8_t dur = 0;     /* ISP clock duration */

/* delays dur CPU clocks */
void spiClockDelay(void) {
  if (dur) {
    _delay_loop_1(dur);
  }
}

/* set dur */
void spiSetSckDuration(uint8_t n) {
  dur = (n + 2) / 3;
}

/* Initialize software SPI interface */
void spiInit(void) {
  /* input: MISO, output: SCK, MOSI, RESET, */
  DDRSPI = (DDRSPI & ~ _BV(MISO)) | _BV(SCK) | _BV(MOSI) | _BV(RESET);

  /* up: RESET, low: MOSI, MISO (pull up), SCK */
  PORTSPI = (PORTSPI & ~ (_BV(MOSI) | _BV(MISO) | _BV(SCK))) | _BV(RESET);
}

/* Close software SPI interface */
void spiClose(void) {
  // TODO
}

/* Switch on/off reset line
   - r: 0 = MCU reset line on, other = off  */
void spiReset(uint8_t r) {
  // TODO: reset polarity
  PORTSPI = r ? PORTSPI | _BV(RESET) : PORTSPI & ~ _BV(RESET);
}

/* Switch on/off SPI clock line
   - r: 0 = SPI clock line on, other = off  */
void spiSck(uint8_t c) {
  PORTSPI = c ? PORTSPI | _BV(SCK) : PORTSPI & ~ _BV(SCK);
}

/* Transmit and receive 1 byte
   - transv: value to transmit
   - return value: received byte     */
uint8_t spiTransmit(uint8_t transv) {
  uint8_t i;
  uint8_t recv;

  recv = 0;
  for (i = _BV(7); i; i >>= 1) {
    PORTSPI = transv & i ? PORTSPI | _BV(MOSI) : PORTSPI & ~_BV(MOSI);
    spiClockDelay();

    PORTSPI |= _BV(SCK);
    spiClockDelay();

    PORTSPI &= ~_BV(SCK);
    recv = recv << 1 | (PINSPI >> MISO & 0x01);
  }
  return recv;
}
