/* AVR programmer HD44780 */

#include "misc.h"
#include "lcd.h"

/* LCD rows and columns */
#define LCDROWS 2
#define LCDCOLS 16

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
#define DELAYCLS     delayMs(10)
#define DELAYHOME    delayMs(1.52)
#define DELAYEMODE   delayUs(37)
#define DELAYDONOFF  delayUs(37)
#define DELAYSHIFT   delayUs(37)
#define DELAYFNCSET  delayUs(37)
#define DELAYSETCADD delayUs(37)
#define DELAYSETDADD delayUs(37)

#define DELAYWRTDATA delayUs(37 + 4)

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

void sendNibble(const uint8_t rs, const  uint8_t n);
void sendByte(const uint8_t rs, const uint8_t n);

uint8_t x;                               /* cursor position */
uint8_t y;

uint8_t buf[LCDCOLS][LCDROWS - 1];  /* buffer for scrolling */

/* Initialize LCD */
void lcdInit(void) {
  DDRC = 0x3f;                                  /* PC0-PC5 output */

  delayMs(15);
  sendNibble(0, (CMDFNCSET | _BV(BITDL)) >> 4); /* function set */
  delayMs(5);
  sendNibble(0, (CMDFNCSET | _BV(BITDL)) >> 4); /* function set */
  delayUs(200);
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

  for (x = 0; x < LCDCOLS; x ++) {
    for (y = 0; y < LCDROWS - 1; y ++) {
      buf[x][y] = ' ';
    }
  }

  x = 0;                                        /* set cursor position */
  y = 0;
}

/* Write a character to current position
   break lines and scrolls
   - c: char to write                     */
void lcdWriteChr(const int8_t c) {
  uint8_t i, j;

  sendByte(1, c);
  DELAYWRTDATA;
  if (y > 0) {         /* save chars to buffer, except 1st line */
    buf[x][y - 1] = c;
  }
  if (++ x >= LCDCOLS) {
    if (++ y >= LCDROWS) {
      for (j = 0; j < LCDROWS; j ++) {
        lcdSetPos(0, j);                          /* scrolling */
        for (i = 0; i < LCDCOLS; i ++) {
          sendByte(1, j < LCDROWS - 1 ? buf[i][j] : ' ');
          DELAYWRTDATA;
          if (j < LCDROWS - 1) {
            buf[i][j] = j < LCDROWS - 2 ? buf[i][j + 1] : ' ';
          }
        }
      }
      y = LCDROWS - 1;
    }
    x = 0;
    lcdSetPos(x, y);
  }
}

/* Write a string to current position
   break lines and scrolls
   - c: string to write                   */
void lcdWriteStr(const int8_t *c) {
  while (*c) {
    lcdWriteChr(*c);
    c ++;
  }
}

/* Write a hex value to current position
   break lines and scrolls
   - c: byte value to print               */
void lcdWriteHex(const uint8_t c) {
  int8_t str[3];

  hex(c, str);
  lcdWriteStr(str);
}

/* Set cursor position
   - col: column, 0 based
   - row: row, 0 based                    */
void lcdSetPos(const uint8_t col, const uint8_t row) {
  uint8_t rows[4] = {0, 0x40, 0x14, 0x54};

  if (row < LCDROWS && col < LCDCOLS) {
    sendByte(0, CMDSETDADD | ((rows[row] + col) & (CMDSETDADD - 1)));
    DELAYSETDADD;
    x = col;
    y = row;
  }
}

/* Send a 4 bit nibble to the LCD
   - rs: 0 = command, 1 = data
   - n: value to send, using low 4 bits   */
void sendNibble(const uint8_t rs, const uint8_t n) {
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
  delayUs(tAS);
  PORTLCD |= _BV(EN);
  delayUs(PWEH);
  PORTLCD &= ~_BV(EN);
  delayUs(tcycE - PWEH - tAS);
}

/* Send a byte to the LCD
   - rs: 0 = command, 1 = data
   - n: value to send                     */
void sendByte(const uint8_t rs, const uint8_t n) {
  sendNibble(rs, n >> 4);
  sendNibble(rs, n);
}
