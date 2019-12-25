/* AVR programmer SPI */

#include "misc.h"
#include "spi.h"
#include "lcd.h"  //
#include <util/delay_basic.h>

#define DDRSPI DDRD
#define PORTSPI PORTD
#define PINSPI PIND

#define SCK   PORTD4
#define MISO  PORTD5
#define MOSI  PORTD6
#define RESET PORTD7

static int8_t dur = 0;     /* ISP clock duration */

static uint8_t spiTransmitDelay(uint8_t transv);
static uint8_t spiTransmitNoDelay(uint8_t transv);

/* Transmit and receive 1 byte
   - transv: value to transmit
   - return value: received byte     */
static uint8_t (*spiTransmitP)(uint8_t transv) = &spiTransmitNoDelay;

/* delays dur CPU clocks */
void spiClockDelay(void) {
  if (dur) {
    _delay_loop_1(dur);
  }
}

/* set dur */
void spiSetSckDuration(uint8_t n) {
  dur = (n + 2) / 3;
  spiTransmitP = n ? &spiTransmitDelay : &spiTransmitNoDelay;
}

/* Initialize software SPI interface */
void spiInit(void) {
  /* input: MISO, output: SCK, MOSI, RESET */
  DDRSPI = (DDRSPI & ~ _BV(MISO)) | _BV(SCK) | _BV(MOSI) | _BV(RESET);

  /* low: MOSI, MISO (pull up), SCK, RESET */
  PORTSPI &= ~ (_BV(MOSI) | _BV(MISO) | _BV(SCK) | _BV(RESET));
}

/* Close software SPI interface */
void spiClose(void) {
  /* input: MOSI, MISO, SCK, RESET */
  DDRSPI &= ~ (_BV(MOSI) | _BV(MISO) | _BV(SCK) | _BV(RESET));

  /* low: MOSI (pull up), MISO (pull up), SCK (pull up), RESET (pull up) */
  PORTSPI &= ~ (_BV(MOSI) | _BV(MISO) | _BV(SCK) | _BV(RESET));
}

/* Switch on/off reset line
   - r: 0 = MCU reset line off, other = on  */
void spiReset(uint8_t r) {
  // TODO: reset polarity
  if (r) {
    PORTSPI |= _BV(RESET);
  }
  else {
    PORTSPI &= ~ _BV(RESET);
  }
}

/* Switch on/off SPI clock line
   - r: 0 = SPI clock line off, other = on  */
void spiSck(uint8_t c) {
  if (c) {
    PORTSPI |= _BV(SCK);
  }
  else {
    PORTSPI &= ~ _BV(SCK);
  }
}

/* Switch on/off SPI MOSI line
   - r: 0 = SPI clock line off, other = on  */
void spiMosi(uint8_t m) {
  if (m) {
    PORTSPI |= _BV(MOSI);
  }
  else {
    PORTSPI &= ~ _BV(MOSI);
  }
}

/* Transmit and receive 1 byte with SPI clock delay
   - transv: value to transmit
   - return value: received byte     */
uint8_t spiTransmit(uint8_t transv) {
  return (*spiTransmitP)(transv);
}


/*
   SPI waveforms:
             MSB                                       LSB
            _____ _____ _____ _____ _____ _____ _____ _____
   MOSI  __/_____X_____X_____X_____X_____X_____X_____X_____\___
              :
            __:__ _____ _____ _____ _____ _____ _____ _____
   MISO  __/_____X_____X_____X_____X_____X_____X_____X_____\___
              : :
              :_:    _     _     _     _     _     _     _
    SCK  _____| |___| |___| |___| |___| |___| |___| |___| |____

*/

/* Transmit and receive 1 byte with SPI clock delay
   - transv: value to transmit
   - return value: received byte     */
static uint8_t spiTransmitDelay(uint8_t transv) {
  uint8_t i;
  uint8_t recv;

  recv = 0;
  for (i = _BV(7); i; i >>= 1) {
    spiMosi(transv & i);
    spiClockDelay();
    spiSck(1);

    spiClockDelay();
    recv <<= 1;
    if (PINSPI & _BV(MISO)) {
      recv ++;
    }
    spiSck(0);
  }
  spiMosi(0);

  return recv;
}

/* Transmit and receive 1 byte without SPI clock delay
   - transv: value to transmit
   - return value: received byte     */
static uint8_t spiTransmitNoDelay(uint8_t transv) {
  uint8_t i;
  uint8_t recv;

  recv = 0;
  for (i = _BV(7); i; i >>= 1) {
    spiMosi(transv & i);
    spiSck(1);

    recv <<= 1;
    if (PINSPI & _BV(MISO)) {
      recv ++;
    }
    spiSck(0);
  }
  spiMosi(0);

  return recv;
}
