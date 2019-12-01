/* HD44780 LCD header */

#ifndef __lcd_h__
#define __lcd_h__

void lcdInit(void);
void lcdWriteChr(const int8_t c);
void lcdWriteStr(const int8_t *c);
void lcdWriteHex(const uint8_t c);
void lcdSetPos(const uint8_t col, const uint8_t row);

#endif
