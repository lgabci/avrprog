/* AVR programmer message */

#include "message.h"
#include "serial.h"
#include "prog.h"
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
    for (i = 0; i < msgsize; i ++) {
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

  for (i = 0; i < msgsize; i ++) {
    transmit(msg[i]);
    checksum ^= msg[i];
  }

  transmit(checksum);
}

/* process message */
void processMessage() {
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
	return;
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
	      return;
	    }
	    else {
              msg[1] = STATUS_CMD_FAILED;
              statusReg = STATUS_CMD_FAILED;
	      return;
	    }
            break;
          case PARAM_VADJUST:
            msgsize = 2;
	    if (msg[2] == 50) {         /* only 5.0 volts */
              msg[1] = STATUS_CMD_OK;
              statusReg = STATUS_CMD_OK;
	      return;
	    }
	    else {
              msg[1] = STATUS_CMD_FAILED;
              statusReg = STATUS_CMD_FAILED;
	      return;
	    }
            break;
          case PARAM_OSC_PSCALE:
            msgsize = 2;
            msg[1] = STATUS_CMD_FAILED;     /* // */
            statusReg = STATUS_CMD_FAILED;
	    return;
            break;
          case PARAM_OSC_CMATCH:
            msgsize = 2;
            msg[1] = STATUS_CMD_FAILED;     /* // */
            statusReg = STATUS_CMD_FAILED;
	    return;
            break;
          case PARAM_SCK_DURATION:
            msgsize = 2;
            msg[1] = STATUS_CMD_FAILED;     /* // */
            statusReg = STATUS_CMD_FAILED;
	    return;
            break;
          case PARAM_RESET_POLARITY:
            paramResetPolarity = msg[2];
            msgsize = 2;
            msg[1] = STATUS_CMD_OK;
            statusReg = STATUS_CMD_OK;
	    return;
            break;
          case PARAM_CONTROLLER_INIT:
            paramControllerInit = msg[2];
            msgsize = 2;
            msg[1] = STATUS_CMD_OK;
            statusReg = STATUS_CMD_OK;
	    return;
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
	    return;
            break;
          case PARAM_BUILD_NUMBER_HIGH:
	    msgsize = 3;
	    msg[1] = STATUS_CMD_OK;     /* message status */
	    msg[2] = 1;                 /* parameter value */
	    statusReg = STATUS_CMD_OK;
	    return;
	    break;
	  case PARAM_HW_VER:
	    msgsize = 3;
	    msg[1] = STATUS_CMD_OK;     /* message status */
	    msg[2] = 1;                 /* parameter value */
	    statusReg = STATUS_CMD_OK;
	    return;
	    break;
	  case PARAM_SW_MAJOR:
	    msgsize = 3;
	    msg[1] = STATUS_CMD_OK;     /* message status */
	    msg[2] = 0;                 /* parameter value */
	    statusReg = STATUS_CMD_OK;
	    return;
	    break;
	  case PARAM_SW_MINOR:
	    msgsize = 3;
	    msg[1] = STATUS_CMD_OK;     /* message status */
	    msg[2] = 1;                 /* parameter value */
	    statusReg = STATUS_CMD_OK;
	    return;
	    break;
	  case PARAM_VTARGET:
            msgsize = 3;
            msg[1] = STATUS_CMD_OK;     /* message status */
            msg[2] = 50;                /* parameter value */
            statusReg = STATUS_CMD_OK;
	    return;
            break;
          case PARAM_VADJUST:
            msgsize = 3;
            msg[1] = STATUS_CMD_OK;     /* message status */
            msg[2] = 50;                /* parameter value */
            statusReg = STATUS_CMD_OK;
	    return;
            break;
          case PARAM_OSC_PSCALE:
            msgsize = 3;
            msg[1] = STATUS_CMD_OK;     /* message status */
            msg[2] = 1;    /* // */     /* parameter value */
            statusReg = STATUS_CMD_OK;
	    return;
            break;
          case PARAM_OSC_CMATCH:
            msgsize = 3;
            msg[1] = STATUS_CMD_OK;     /* message status */
            msg[2] = 1;    /* // */     /* parameter value */
            statusReg = STATUS_CMD_OK;
	    return;
            break;
          case PARAM_SCK_DURATION:
            msgsize = 3;
            msg[1] = STATUS_CMD_OK;     /* message status */
            msg[2] = 1;    /* // */     /* parameter value */
            statusReg = STATUS_CMD_OK;
	    return;
            break;
          case PARAM_TOPCARD_DETECT:
            msgsize = 3;
            msg[1] = STATUS_CMD_OK;     /* message status */
            msg[2] = 0;                 /* parameter value */
            statusReg = STATUS_CMD_OK;
	    return;
            break;
          case PARAM_STATUS:
            msgsize = 3;
            msg[1] = STATUS_CMD_OK;     /* message status */
            msg[2] = statusReg;         /* parameter value */
            statusReg = STATUS_CMD_OK;
	    return;
            break;
          case PARAM_DATA:
            msgsize = 3;
            msg[1] = STATUS_CMD_OK;     /* message status */
            msg[2] = 0;                 /* parameter value */
            statusReg = STATUS_CMD_OK;
	    return;
            break;
          case PARAM_RESET_POLARITY:
            msgsize = 3;
            msg[1] = STATUS_CMD_OK;     /* message status */
            msg[2] = paramResetPolarity;  /* parameter value */
            statusReg = STATUS_CMD_OK;
	    return;
            break;
          case PARAM_CONTROLLER_INIT:
            msgsize = 3;
            msg[1] = STATUS_CMD_OK;     /* message status */
            msg[2] = paramControllerInit;  /* parameter value */
            statusReg = STATUS_CMD_OK;
	    return;
            break;
        }
      }
      break;
    case CMD_OSCCAL:               /* ------------------------------------ */
      if (msgsize == 1) {
        msgsize = 2;     /* // */
        msg[1] = STATUS_CMD_OK;
        statusReg = STATUS_CMD_FAILED;
	return;
      }
      break;
    case CMD_LOAD_ADDRESS:         /* ------------------------------------ */
      if (msgsize == 5) {
        msgsize = 2;     /* // */
        msg[1] = STATUS_CMD_OK;
        statusReg = msg[1];
	return;
      }
    case CMD_FIRMWARE_UPGRADE:     /* ------------------------------------ */
      if (msgsize == 11) {
        msgsize = 2;     /* // */
        msg[1] = STATUS_CMD_FAILED;
        statusReg = STATUS_CMD_FAILED;
	return;
      }
    case CMD_ENTER_PROGMODE_ISP:   /* ------------------------------------ */
      if (msgsize == 12) {
        msgsize = 2;     /* // */
        msg[1] = enterProgMode(msg[1], msg[2], msg[3], msg[4], msg[5],
          msg[6], msg[7], msg[8], msg[9], msg[10], msg[11]);
        statusReg = msg[1];
	return;
      }
      break;
    case CMD_LEAVE_PROGMODE_ISP:   /* ------------------------------------ */
      if (msgsize == 3) {
        msgsize = 2;
        msg[1] = STATUS_CMD_OK;
        statusReg = msg[1];
	return;
      }
      break;


    default:
      break;
  }

  msgsize = 2;                      /* error on unknown command */
  msg[1] = STATUS_CMD_FAILED;
  statusReg = STATUS_CMD_FAILED;
}
