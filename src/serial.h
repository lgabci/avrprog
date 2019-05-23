/* AVR programmer serial header */

#ifndef __serial_h__
#define __serial_h__

void initUSART(void);
unsigned char receive(unsigned char *c);
void transmit(unsigned char c);

#endif
