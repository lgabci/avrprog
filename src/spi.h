/* AVR programmer SPI header */

#ifndef __spi_h__
#define __spi_h__

#include "misc.h"

void spiInit(void);
void spiClose(void);
void spiReset(uint8_t r);
void spiSck(uint8_t c);
uint8_t spiTransmit(uint8_t transv);

#endif
