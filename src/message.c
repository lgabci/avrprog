/* AVR programmer message */

#include "misc.h"
#include "message.h"
#include "serial.h"
#include "prog.h"
#include "command.h"
#include "lcd.h"  /////

#define MAXMSGSIZE 100
uint8_t seq;              /* message sequence    */
uint16_t msgSize;         /* message size in msg */
uint8_t msg[MAXMSGSIZE];  /* message body        */

/* PARAM_RESET_POLARITY
     0 = active high reset (AT89)
     1 = active low reset (AVR) */
uint8_t paramResetPolarity = 1;
/* PARAM_CONTROLLER_INIT
     set 0 at reset, host can set it, and test if the power has been lost */
uint8_t paramControllerInit = 0;

uint8_t statusReg;

/* reads a message */
void readMessage() {
  uint8_t c;
  uint8_t chksum;
  uint16_t i;

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

  while (1) {
    /* start */
    chksum = 0;
    if (! usartReceive(&c)) {
      continue;
    }
    if (c != MESSAGE_START) {
      continue;
    }
    chksum ^= c;

    /* get seq */
    if (! usartReceive(&seq)) {
      continue;
    }
    chksum ^= seq;

    /* get msg size 1 */
    if (! usartReceive(&c)) {
      continue;
    }
    chksum ^= c;
    msgSize = (uint16_t)c << 8;

    /* get msg size 2 */
    if (! usartReceive(&c)) {
      continue;
    }
    chksum ^= c;
    msgSize |= c;

    /* get token */
    if (! usartReceive(&c)) {
      continue;
    }
    if (c != TOKEN) {
      continue;
    }
    chksum ^= c;

    /* get data, read even if it is too long */
    for (i = 0; i < msgSize; i ++) {
      if (! usartReceive(&c)) {
        continue;
      }
      chksum ^= c;

      if (msgSize <= MAXMSGSIZE) {  /* store message if it is not too long */
        msg[i] = c;
      }
    }

    /* get checksum */
    if (! usartReceive(&c)) {
      continue;
    }
    if (c != chksum) {
      continue;
    }
    if (msgSize > MAXMSGSIZE) {
      msg[i] = c;
    }

    break;
  }
}

/* send message */
void sendMessage() {
  uint8_t checksum;
  uint16_t i;

/* parameter        size  description
   MSG_START           1  always 0x1B
   SEQ_NUMBER          1  incremented for each message, wraps after 0xFF
   MSG_SIZE            2  MSB first, size of message body
   TOKEN               1  always 0x0E
   MSG_BODY     MSG_SIZE  message body, from 0 to 65535 bytes
   CHECKSUM            1  XOR all bytes in message                         */

  checksum = 0;
  usartTransmit(MESSAGE_START);
  checksum ^= MESSAGE_START;

  usartTransmit(seq);
  checksum ^= seq;

  usartTransmit(msgSize >> 8);
  checksum ^= msgSize >> 8;

  usartTransmit(msgSize & 0xff);
  checksum ^= msgSize & 0xff;

  usartTransmit(TOKEN);
  checksum ^= TOKEN;

  for (i = 0; i < msgSize; i ++) {
    usartTransmit(msg[i]);
    checksum ^= msg[i];
  }

  usartTransmit(checksum);
}

/* process message */
void processMessage() {
  switch(msg[0]) {
    case CMD_SIGN_ON:               /* ----------------------------------- */
      if (msgSize == 1) {
        msgSize = 11;
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
      if (msgSize == 3) {
        switch (msg[1]) {
          case PARAM_VTARGET:
            msgSize = 2;
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
            msgSize = 2;
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
            msgSize = 2;
            msg[1] = STATUS_CMD_FAILED;     /* // */
            statusReg = STATUS_CMD_FAILED;
            return;
            break;
          case PARAM_OSC_CMATCH:
            msgSize = 2;
            msg[1] = STATUS_CMD_FAILED;     /* // */
            statusReg = STATUS_CMD_FAILED;
            return;
            break;
          case PARAM_SCK_DURATION:
            msgSize = 2;
            msg[1] = STATUS_CMD_FAILED;     /* // */
            statusReg = STATUS_CMD_FAILED;
            return;
            break;
          case PARAM_RESET_POLARITY:
            paramResetPolarity = msg[2];
            msgSize = 2;
            msg[1] = STATUS_CMD_OK;
            statusReg = STATUS_CMD_OK;
            return;
            break;
          case PARAM_CONTROLLER_INIT:
            paramControllerInit = msg[2];
            msgSize = 2;
            msg[1] = STATUS_CMD_OK;
            statusReg = STATUS_CMD_OK;
            return;
            break;
        }
      }
      break;
    case CMD_GET_PARAMETER:         /* ----------------------------------- */
      if (msgSize == 2) {
        switch (msg[1]) {
          case PARAM_BUILD_NUMBER_LOW:
            msgSize = 3;
            msg[1] = STATUS_CMD_OK;     /* message status */
            msg[2] = 0;                 /* parameter value */
            statusReg = STATUS_CMD_OK;
            return;
            break;
          case PARAM_BUILD_NUMBER_HIGH:
            msgSize = 3;
            msg[1] = STATUS_CMD_OK;     /* message status */
            msg[2] = 1;                 /* parameter value */
            statusReg = STATUS_CMD_OK;
            return;
            break;
          case PARAM_HW_VER:
            msgSize = 3;
            msg[1] = STATUS_CMD_OK;     /* message status */
            msg[2] = 1;                 /* parameter value */
            statusReg = STATUS_CMD_OK;
            return;
            break;
          case PARAM_SW_MAJOR:
            msgSize = 3;
            msg[1] = STATUS_CMD_OK;     /* message status */
            msg[2] = 0;                 /* parameter value */
            statusReg = STATUS_CMD_OK;
            return;
            break;
          case PARAM_SW_MINOR:
            msgSize = 3;
            msg[1] = STATUS_CMD_OK;     /* message status */
            msg[2] = 1;                 /* parameter value */
            statusReg = STATUS_CMD_OK;
            return;
            break;
          case PARAM_VTARGET:
            msgSize = 3;
            msg[1] = STATUS_CMD_OK;     /* message status */
            msg[2] = 50;                /* parameter value */
            statusReg = STATUS_CMD_OK;
            return;
            break;
          case PARAM_VADJUST:
            msgSize = 3;
            msg[1] = STATUS_CMD_OK;     /* message status */
            msg[2] = 50;                /* parameter value */
            statusReg = STATUS_CMD_OK;
            return;
            break;
          case PARAM_OSC_PSCALE:
            msgSize = 3;
            msg[1] = STATUS_CMD_OK;     /* message status */
            msg[2] = 1;    /* // */     /* parameter value */
            statusReg = STATUS_CMD_OK;
            return;
            break;
          case PARAM_OSC_CMATCH:
            msgSize = 3;
            msg[1] = STATUS_CMD_OK;     /* message status */
            msg[2] = 1;    /* // */     /* parameter value */
            statusReg = STATUS_CMD_OK;
            return;
            break;
          case PARAM_SCK_DURATION:
            msgSize = 3;
            msg[1] = STATUS_CMD_OK;     /* message status */
            msg[2] = 1;    /* // */     /* parameter value */
            statusReg = STATUS_CMD_OK;
            return;
            break;
          case PARAM_TOPCARD_DETECT:
            msgSize = 3;
            msg[1] = STATUS_CMD_OK;     /* message status */
            msg[2] = 0;                 /* parameter value */
            statusReg = STATUS_CMD_OK;
            return;
            break;
          case PARAM_STATUS:
            msgSize = 3;
            msg[1] = STATUS_CMD_OK;     /* message status */
            msg[2] = statusReg;         /* parameter value */
            statusReg = STATUS_CMD_OK;
            return;
            break;
          case PARAM_DATA:
            msgSize = 3;
            msg[1] = STATUS_CMD_OK;     /* message status */
            msg[2] = 0;                 /* parameter value */
            statusReg = STATUS_CMD_OK;
            return;
            break;
          case PARAM_RESET_POLARITY:
            msgSize = 3;
            msg[1] = STATUS_CMD_OK;     /* message status */
            msg[2] = paramResetPolarity;  /* parameter value */
            statusReg = STATUS_CMD_OK;
            return;
            break;
          case PARAM_CONTROLLER_INIT:
            msgSize = 3;
            msg[1] = STATUS_CMD_OK;     /* message status */
            msg[2] = paramControllerInit;  /* parameter value */
            statusReg = STATUS_CMD_OK;
            return;
            break;
        }
      }
      break;
    case CMD_OSCCAL:               /* ------------------------------------ */
      if (msgSize == 1) {
        msgSize = 2;     /* // */
        msg[1] = STATUS_CMD_OK;
        statusReg = STATUS_CMD_FAILED;
        return;
      }
      break;
    case CMD_LOAD_ADDRESS:         /* ------------------------------------ */
      if (msgSize == 5) {
        msgSize = 2;     /* // */
        msg[1] = loadAddress(&msg[1]);
        statusReg = msg[1];
        return;
      }
    case CMD_FIRMWARE_UPGRADE:     /* ------------------------------------ */
      if (msgSize == 11) {
        msgSize = 2;     /* // */
        msg[1] = STATUS_CMD_FAILED;
        statusReg = STATUS_CMD_FAILED;
        return;
      }
    case CMD_ENTER_PROGMODE_ISP:   /* ------------------------------------ */
      if (msgSize == 12) {
        msgSize = 2;     /* // */
        msg[1] = enterProgModeIsp(msg[1], msg[2], msg[3], msg[4], msg[5],
          msg[6], msg[7], &msg[8]);
        statusReg = msg[1];
        return;
      }
      break;
    case CMD_LEAVE_PROGMODE_ISP:   /* ------------------------------------ */
      if (msgSize == 3) {
        msgSize = 2;               /* // */
        msg[1] = STATUS_CMD_OK;
        statusReg = msg[1];
        return;
      }
      break;
    case CMD_CHIP_ERASE_ISP:       /* ------------------------------------ */
      if (msgSize == 7) {
        msgSize = 2;               /* // */
        msg[1] = STATUS_CMD_OK;
        statusReg = msg[1];
        return;
      }
      break;
    case CMD_PROGRAM_FLASH_ISP:    /* ------------------------------------ */
      if (msgSize >= 10) {
        uint16_t numOfBytes;

        numOfBytes = (uint16_t)msg[1] << 8 | msg[2];
        if (msgSize == numOfBytes + 10) {
          msgSize = 2;               /* // */
          msg[1] = STATUS_CMD_OK;
          statusReg = msg[1];
          return;
        }
      }
      break;


    case CMD_READ_FLASH_ISP:       /* ------------------------------------ */
      if (msgSize == 4) {
        uint16_t blockSize;

        blockSize = (uint16_t)msg[1] << 8 | msg[2];
        msg[1] = readFlashIsp(blockSize, msg[3], &msg[2]);
        if (msg[1] == STATUS_CMD_OK) {
          msgSize = blockSize + 3;
          msg[blockSize + 2] = msg[1];
        }
        statusReg = msg[1];
        return;
      }
      break;

    case CMD_READ_FUSE_ISP:        /* ------------------------------------ */
    case CMD_READ_SIGNATURE_ISP:   /* ------------------------------------ */
      if (msgSize == 6) {
        msgSize = 4;
        msg[1] = readFuseIsp(msg[1], &msg[2], &msg[2]);
        statusReg = msg[1];
        msg[3] = msg[1];
        return;
      }
      break;
  }

  lcdWriteHex(msg[0]);  ///
  // lcdWriteChr(' ');     ///
  msgSize = 2;                    /* unknown command or bad message */
  msg[1] = STATUS_CMD_FAILED;
  statusReg = msg[1];
  return;
}
