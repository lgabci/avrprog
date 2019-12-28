/* AVR programmer LED header */

#ifndef __led_h__
#define __led_h__

void ledInit(void);
void ledOk(void);
void ledError(void);
void ledSetProcessing(void);
void ledClrProcessing(void);
void ledSetTx(void);
void ledClrTx(void);

#endif

