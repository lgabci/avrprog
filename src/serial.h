/* AVR programmer serial header */

#ifndef __serial_h__
#define __serial_h__

void usartInit(void);
uint8_t usartReceive(uint8_t *c);
void usartTransmit(uint8_t c);

#endif
