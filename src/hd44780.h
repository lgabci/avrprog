/* HD44780 header */

#ifndef __hd44780_h_
#define __hd44780_h_

void initLcd(void);
void writeChr(const char c);
void writeStr(const char *c);
void setPos(const unsigned char col, const unsigned char row);

#endif
