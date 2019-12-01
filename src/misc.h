/* AVR programmer misc header */

#ifndef __misc_h__
#define __misc_h__

#include <stdint.h>
#include <avr/io.h>

#define F_CPU 7372800UL

void emClockInit(void);
void hex(const uint8_t v, int8_t *str);
void delayMs(uint8_t w);
void delayUs(uint8_t w);

#endif
