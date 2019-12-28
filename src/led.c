/* AVR programmer LED */

#include "misc.h"
#include "led.h"

#define LEDPORT PORTC
#define LEDDDR  DDRC

#define LEDOK   PORTC5
#define LEDPRC  PORTC4
#define LEDERR  PORTC3
#define LEDTRN  PORTC2

void ledInit(void) {
  LEDDDR |= _BV(LEDOK) | _BV(LEDPRC) | _BV(LEDERR);
  LEDPORT &= ~ (_BV(LEDOK) | _BV(LEDPRC) | _BV(LEDERR) | _BV(LEDTRN));
}

void ledOk(void) {
  LEDPORT &= ~ (_BV(LEDPRC) | _BV(LEDERR) | _BV(LEDTRN));
  LEDPORT |= _BV(LEDOK);
}

void ledError(void) {
  LEDPORT &= ~ (_BV(LEDOK) | _BV(LEDPRC) | _BV(LEDTRN));
  LEDPORT |= _BV(LEDERR);
}

void ledSetProcessing(void) {  /* don't clear Ok or Error */
  LEDPORT |= _BV(LEDPRC);
}

void ledClrProcessing(void) {  /* don't clear Ok or Error */
  LEDPORT &= ~ _BV(LEDPRC);
}

void ledSetTx(void) {  /* don't clear Ok or Error */
  LEDPORT |= _BV(LEDTRN);
}

void ledClrTx(void) {  /* don't clear Ok or Error */
  LEDPORT &= ~ _BV(LEDTRN);
}
