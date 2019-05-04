#define F_CPU 7372800UL

#include <avr/io.h>
#include <util/delay.h>

#include "command.h"

#define MAXMSGSIZE 100
unsigned char seq;              /* message sequence    */
unsigned short int msgsize;     /* message size in msg */
unsigned char msg[MAXMSGSIZE];  /* message body        */

/* PARAM_RESET_POLARITY
     0 = active high reset (AT89)
     1 = active low reset (AVR) */
unsigned char paramResetPolarity = 1;
/* PARAM_CONTROLLER_INIT
     set 0 at reset, host can set it, and test if the power has been lost */
unsigned char paramControllerInit = 0;

unsigned char statusReg = STATUS_CMD_OK;

void initUSART(void) {
/* The STK500 uses: 115.2 kbps, 8 data bits, 1 stop bit, no parity
   UDR    ----------------------- UART data --------------------------
   UCSRA  RXC     TXC     UDRE    FE      DOR     PE      U2X     MPCM
   UCSRB  RXCIE   TXCIE   UDRIE   RXEN    TXEN    UCSZ2   RXB8    TXB8
   UCSRC  URSEL   UMSEL   UPM1    UPM0    USBS    UCSZ1   UCSZ0   UCPOL
   UBRRH  URSEL   -       -       -       UBRR3   UBRR2   UBRR1   UBRR0
   UBRRL  UBBR7   UBBR6   UBBR5   UBBR4   UBBR3   UBBR2   UBBR1   UBBR0   */

#define BAUD 115200
  UBRRH = (F_CPU / 16 / BAUD - 1) >> 8;
  UBRRL = F_CPU / 16 / BAUD - 1;
#undef BAUD
  UCSRA = 0;
  UCSRC = _BV(URSEL) | _BV(UCSZ1) | _BV(UCSZ0);
  UCSRB = _BV(RXEN) | _BV(TXEN);
}

/* // ---------------------------------------------------------------- */
void blink(unsigned char count) {
  unsigned char i;
  for(i = 0; i < count * 2; i ++) {
  PORTC ^= _BV(PC5);
  _delay_ms(500);
  }
}
/* ---------------------------------------------------------------- */

/* Receive a character from USART, returns error */
unsigned char receive(unsigned char *c) {
  unsigned char err;

  while (! (UCSRA & _BV(RXC)))
    ;
  err = UCSRA & (_BV(FE) | _BV(DOR));
  *c = UDR;
  return ! err;
}

/* Send a character on USART */
void transmit(unsigned char c) {
  while (! (UCSRA & _BV(UDRE)));
  UDR = c;
}

/* reads a message */
void readMessage() {
  unsigned char c;
  unsigned char chksum;
  unsigned short int i;

/* parameter        size  description
   MSG_START           1  always 0x1B
   SEQ_NUMBER          1  incremented for each message, wraps after 0xFF
   MSG_SIZE            2  MSB first, size of message body
   TOKEN               1  always 0x0E
   MSG_BODY     MSG_SIZE  message body, from 0 to 65535 bytes
   CHECKSUM            1  XOR all bytes in message

   state table
   current state    event      condition                 next state

   start            read char  char == 0x1B              get seq
                    read char  char != 0x1B              start

   get seq          read char  store seq                 get msg size1

   get msg size1    read char  store size1               get msg size2

   get msg size2    read char  store size2               get token

   get token        read char  char == 0x0E              get data
                    read char  char != 0x0E              start

   get data         read char  msg size reached          get checksum
                    read char  msg size not reached      get data

   get checksum     read char  checksum OK
                    read char  checksum NOT OK           start             */

#define RECV(c) \
    if (! receive(&(c))) { \
      continue; \
    }

  while (1) {
    /* start */
    chksum = 0;
    RECV(c)
    if (c != MESSAGE_START) {
      continue;
    }
    chksum ^= c;

    /* get seq */
    RECV(seq)
    chksum ^= seq;

    /* get msg size 1 */
    RECV(c)
    chksum ^= c;
    msgsize = (unsigned short int)c << 8;

    /* get msg size 2 */
    RECV(c)
    chksum ^= c;
    msgsize += c;

    /* get token */
    RECV(c)
    if (c != TOKEN) {
      continue;
    }
    chksum ^= c;

    /* get data, readars even if it is too long */
    for(i = 0; i < msgsize; i ++) {
      RECV(c)
      chksum ^= c;

      if (msgsize <= MAXMSGSIZE) {  /* store message if it is not too long */
        msg[i] = c;
      }
    }

    /* get checksum */
    RECV(c)
    if (c != chksum) {
      continue;
    }
    if (msgsize > MAXMSGSIZE) {
      msg[i] = c;
    }

    break;
  }
#undef RECV
}

/* process message */
void processMessage() {
  unsigned char ok = 0;

  switch(msg[0]) {
    case CMD_SIGN_ON:               /* ----------------------------------- */
      if (msgsize == 1) {
        msgsize = 11;
        msg[1] = STATUS_CMD_OK;     /* message status */
	msg[2] = 8;                 /* length of signature string */
	msg[3] = 'S';
	msg[4] = 'T';
	msg[5] = 'K';
	msg[6] = '5';
	msg[7] = '0';
	msg[8] = '0';
	msg[9] = '_';
	msg[10] = '2';
        statusReg = STATUS_CMD_OK;
	ok = 1;
      }
      break;
    case CMD_SET_PARAMETER:         /* ----------------------------------- */
      if (msgsize == 3) {
        switch (msg[1]) {
	  case PARAM_VTARGET:
            msgsize = 2;
	    if (msg[2] == 50) {         /* only 5.0 volts */
              msg[1] = STATUS_CMD_OK;
              statusReg = STATUS_CMD_OK;
	      ok = 1;
	    }
	    else {
              msg[1] = STATUS_CMD_FAILED;
              statusReg = STATUS_CMD_FAILED;
	      ok = 1;
	    }
            break;
          case PARAM_VADJUST:
            msgsize = 2;
	    if (msg[2] == 50) {         /* only 5.0 volts */
              msg[1] = STATUS_CMD_OK;
              statusReg = STATUS_CMD_OK;
	      ok = 1;
	    }
	    else {
              msg[1] = STATUS_CMD_FAILED;
              statusReg = STATUS_CMD_FAILED;
	      ok = 1;
	    }
            break;
          case PARAM_OSC_PSCALE:
            msgsize = 2;
            msg[1] = STATUS_CMD_FAILED;     /* // */
            statusReg = STATUS_CMD_FAILED;
	    ok = 1;
            break;
          case PARAM_OSC_CMATCH:
            msgsize = 2;
            msg[1] = STATUS_CMD_FAILED;     /* // */
            statusReg = STATUS_CMD_FAILED;
	    ok = 1;
            break;
          case PARAM_SCK_DURATION:
            msgsize = 2;
            msg[1] = STATUS_CMD_FAILED;     /* // */
            statusReg = STATUS_CMD_FAILED;
	    ok = 1;
            break;
          case PARAM_CONTROLLER_INIT:
            msgsize = 2;
            msg[1] = STATUS_CMD_OK;     /* message status */
            paramControllerInit = msg[2];  /* parameter value */
            statusReg = STATUS_CMD_OK;
	    ok = 1;
            break;
	}
      }
      break;
    case CMD_GET_PARAMETER:         /* ----------------------------------- */
      if (msgsize == 2) {
        switch (msg[1]) {
          case PARAM_BUILD_NUMBER_LOW:
            msgsize = 3;
            msg[1] = STATUS_CMD_OK;     /* message status */
            msg[2] = 0;                 /* parameter value */
            statusReg = STATUS_CMD_OK;
	    ok = 1;
            break;
          case PARAM_BUILD_NUMBER_HIGH:
	    msgsize = 3;
	    msg[1] = STATUS_CMD_OK;     /* message status */
	    msg[2] = 1;                 /* parameter value */
	    statusReg = STATUS_CMD_OK;
	    ok = 1;
	    break;
	  case PARAM_HW_VER:
	    msgsize = 3;
	    msg[1] = STATUS_CMD_OK;     /* message status */
	    msg[2] = 1;                 /* parameter value */
	    statusReg = STATUS_CMD_OK;
	    ok = 1;
	    break;
	  case PARAM_SW_MAJOR:
	    msgsize = 3;
	    msg[1] = STATUS_CMD_OK;     /* message status */
	    msg[2] = 0;                 /* parameter value */
	    statusReg = STATUS_CMD_OK;
	    ok = 1;
	    break;
	  case PARAM_SW_MINOR:
	    msgsize = 3;
	    msg[1] = STATUS_CMD_OK;     /* message status */
	    msg[2] = 1;                 /* parameter value */
	    statusReg = STATUS_CMD_OK;
	    ok = 1;
	    break;
	  case PARAM_VTARGET:
            msgsize = 3;
            msg[1] = STATUS_CMD_OK;     /* message status */
            msg[2] = 50;                /* parameter value */
            statusReg = STATUS_CMD_OK;
	    ok = 1;
            break;
          case PARAM_VADJUST:
            msgsize = 3;
            msg[1] = STATUS_CMD_OK;     /* message status */
            msg[2] = 50;                /* parameter value */
            statusReg = STATUS_CMD_OK;
	    ok = 1;
            break;
          case PARAM_OSC_PSCALE:
            msgsize = 3;
            msg[1] = STATUS_CMD_OK;     /* message status */
            msg[2] = 1;    /* // */     /* parameter value */
            statusReg = STATUS_CMD_OK;
	    ok = 1;
            break;
          case PARAM_OSC_CMATCH:
            msgsize = 3;
            msg[1] = STATUS_CMD_OK;     /* message status */
            msg[2] = 1;    /* // */     /* parameter value */
            statusReg = STATUS_CMD_OK;
	    ok = 1;
            break;
          case PARAM_SCK_DURATION:
            msgsize = 3;
            msg[1] = STATUS_CMD_OK;     /* message status */
            msg[2] = 1;    /* // */     /* parameter value */
            statusReg = STATUS_CMD_OK;
	    ok = 1;
            break;
          case PARAM_TOPCARD_DETECT:
            msgsize = 3;
            msg[1] = STATUS_CMD_OK;     /* message status */
            msg[2] = 0;                 /* parameter value */
            statusReg = STATUS_CMD_OK;
	    ok = 1;
            break;
          case PARAM_STATUS:
            msgsize = 3;
            msg[1] = STATUS_CMD_OK;     /* message status */
            msg[2] = statusReg;         /* parameter value */
            statusReg = STATUS_CMD_OK;
	    ok = 1;
            break;
          case PARAM_DATA:
            msgsize = 3;
            msg[1] = STATUS_CMD_OK;     /* message status */
            msg[2] = 0;                 /* parameter value */
            statusReg = STATUS_CMD_OK;
	    ok = 1;
            break;
          case PARAM_RESET_POLARITY:
            msgsize = 3;
            msg[1] = STATUS_CMD_OK;     /* message status */
            msg[2] = paramResetPolarity;  /* parameter value */
            statusReg = STATUS_CMD_OK;
	    ok = 1;
            break;
          case PARAM_CONTROLLER_INIT:
            msgsize = 3;
            msg[1] = STATUS_CMD_OK;     /* message status */
            msg[2] = paramControllerInit;  /* parameter value */
            statusReg = STATUS_CMD_OK;
	    ok = 1;
            break;
        }
      }
      break;



    case CMD_ENTER_PROGMODE_ISP:   /* ------------------------------------ */
      if (msgsize == 12) {
        msgsize = 2;     /* // */
        msg[1] = STATUS_CMD_OK;
        statusReg = STATUS_CMD_OK;
	ok = 1;
      }
      break;
    case CMD_LEAVE_PROGMODE_ISP:   /* ------------------------------------ */
      if (msgsize == 3) {
        msgsize = 2;
        msg[1] = STATUS_CMD_OK;
        statusReg = STATUS_CMD_OK;
	ok = 1;
      }
      break;



    default:
      blink(1);  /* // */
      break;
  }
  if (! ok) {
    msgsize = 2;                      /* error on unknown command */
    msg[1] = STATUS_CMD_FAILED;
    statusReg = STATUS_CMD_FAILED;
  }
}

/* send message */
void sendMessage() {
  unsigned char checksum;
  unsigned short int i;

/* parameter        size  description
   MSG_START           1  always 0x1B
   SEQ_NUMBER          1  incremented for each message, wraps after 0xFF
   MSG_SIZE            2  MSB first, size of message body
   TOKEN               1  always 0x0E
   MSG_BODY     MSG_SIZE  message body, from 0 to 65535 bytes
   CHECKSUM            1  XOR all bytes in message                         */

  checksum = 0;
  transmit(MESSAGE_START);
  checksum ^= MESSAGE_START;

  transmit(seq);
  checksum ^= seq;

  transmit(msgsize >> 8);
  checksum ^= msgsize >> 8;

  transmit(msgsize & 0xff);
  checksum ^= msgsize & 0xff;

  transmit(TOKEN);
  checksum ^= TOKEN;

  for(i = 0; i < msgsize; i ++) {
    transmit(msg[i]);
    checksum ^= msg[i];
  }

  transmit(checksum);
}

int main() {
/* // ---------------------------------------------- */
  DDRC |= _BV(PC5);
/* ---------------------------------------------- */
  initUSART();

  while (1) {
    readMessage();
    processMessage();
    sendMessage();
  }

  return 0;
}
