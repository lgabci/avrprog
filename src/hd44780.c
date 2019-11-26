/* AVR programmer HD44780 */

#include "common.h"

#include <avr/io.h>
#include <util/delay.h>

#include "hd44780.h"

/* used port */
#define PORTLCD PORTC

#define B0 PC0
#define B1 PC1
#define B2 PC2
#define B3 PC3
#define RS PC4
#define EN PC5

/* commands */
#define CMDCLS     0x01  /* clear display           */
#define CMDHOME    0x02  /* return home             */
#define CMDEMODE   0x04  /* entry mode set          */
#define CMDDONOFF  0x08  /* display on/off control  */
#define CMDSHIFT   0x10  /* cursor or display shift */
#define CMDFNCSET  0x20  /* function set            */
#define CMDSETCADD 0x40  /* set CGRAM address       */
#define CMDSETDADD 0x80  /* set DDRAM address       */

/* waiting for commands to complete */
#define DELAYCLS     _delay_ms(10)
#define DELAYHOME    _delay_ms(1.52)
#define DELAYEMODE   _delay_us(37)
#define DELAYDONOFF  _delay_us(37)
#define DELAYSHIFT   _delay_us(37)
#define DELAYFNCSET  _delay_us(37)
#define DELAYSETCADD _delay_us(37)
#define DELAYSETDADD _delay_us(37)

#define DELAYWRTDATA _delay_us(37 + 4)

/* bit positions for commands */
#define BITS  0 /* 0 = don't shift, 1 = shift */
#define BITID 1 /* 0 = dec, 1 = inc */
#define BITB  0 /* screen 0 = off, 1 = on */
#define BITC  1 /* cursor 0 = off, 1 = on */
#define BITD  2 /* blink  0 = off, 1 = on */
#define BITRL 2 /* 0 = left, 1 = right */
#define BITSC 3 /* 0 = cursor move, 1 = display shift */
#define BITF  2 /* font: 0 = 5x8 dot, 1 = 5x10 dots */
#define BITN  3 /* 0 = 1 line, 1 = 2 lines */
#define BITDL 4 /* 0 = 4 bits, 1 = 8 bits */

/* timing values in microseconds */
#define tcycE 0.500
#define PWEH  0.230
#define tAS   0.40
#define tAH   0.10
#define tDSW  0.80
#define tH    0.10

void sendNibble(const unsigned char rs, const  unsigned char n);
void sendByte(const unsigned char rs, const unsigned char n);

void initLcd(void) {
  DDRC = 0x3f;					/* PC0-PC5 output */

  _delay_ms(15);
  sendNibble(0, (CMDFNCSET | _BV(BITDL)) >> 4); /* function set */
  _delay_ms(5);
  sendNibble(0, (CMDFNCSET | _BV(BITDL)) >> 4); /* function set */
  _delay_us(200);
  sendNibble(0, (CMDFNCSET | _BV(BITDL)) >> 4); /* function set */
  DELAYFNCSET;
  sendNibble(0, (CMDFNCSET) >> 4); /* function set: 4 bit interface */
  DELAYFNCSET;
  sendByte(0, CMDFNCSET | _BV(BITN) ); /* f. set: 4 bit, 2 lines, 5x8 dot */
  DELAYFNCSET;
  sendByte(0, CMDDONOFF);                        /* display off */
  DELAYDONOFF;
  sendByte(0, CMDCLS);                          /* clear screen */
  DELAYCLS;
  sendByte(0, CMDEMODE | _BV(BITID));    /* entry mode set, don't shift */
  DELAYEMODE;

  sendByte(0, CMDDONOFF | _BV(BITD) | _BV(BITC) | _BV(BITB)); /* display on */
  DELAYDONOFF;
}

void writeChr(const char c) {
  sendByte(1, c);
  DELAYWRTDATA;
}

void writeStr(const char *c) {
  while (*c) {
    writeChr(*c);
    c ++;
  }
}

void setPos(const unsigned char col, const unsigned char row) {
  sendByte(0, CMDSETDADD | (((row << 6) + col) & (CMDSETDADD - 1)));
  DELAYSETDADD;
}

void sendNibble(const unsigned char rs, const unsigned char n) {
/* timing

   Item                                 Symbol     Min     Typ     Max     Unit
   ------------------------------------ ---------- ------- ------- ------- -------
   Enable cycle time                    tcycE      500     -       -       ns
   Enable pulse width (high level)      PWEH       230     -       -
   Enable rise/fall time                tEr, tEf   -       -       20
   Address set-up time (RS, R/W to E)   tAS        40      -       -
   Address hold time                    tAH        10      -       -
   Data set-up time                     tDSW       80      -       -
   Data hold time                       tH         10      -       -
     _
   R/W is always on GND, we don't set it.

             ____ ____________________________________ ___________
          RS ____X____________________________________X___________
                 :                                    :
                 <-- tAS -->                <-- tAH -->
           _ ____:         :                :         :___________
         R/W ____\____________________________________/___________
                 :         :                :
                           <----- PWEH ----->
                           :________________:                 ____
          E  ______________/                \________________/   
                        -->:<-- tEr      -->:<-- tEf         :
                           :     <-- tDSW --x-- tH --->      :
             ____________________:____________________:___________
   DB0 - DB7 ____________________X____________________X___________
                           :                                 :
                           <------------- tcycE ------------->
 */
  PORTLCD = (n & (_BV(B0) | _BV(B1) | _BV(B2) | _BV(B3))) | (rs ? _BV(RS) : 0);
  _delay_us(tAS);
  PORTLCD |= _BV(EN);
  _delay_us(PWEH);
  PORTLCD &= ~_BV(EN);
  _delay_us(tcycE - PWEH - tAS);
}

void sendByte(const unsigned char rs, const unsigned char n) {
  sendNibble(rs, n >> 4);
  sendNibble(rs, n);
}

