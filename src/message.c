/* AVR programmer message */

#include "misc.h"
#include "message.h"
#include "serial.h"
#include "prog.h"
#include "command.h"
#include "lcd.h"  /////

#define MAXMSGSIZE 100
static uint8_t seq;              /* message sequence    */
static uint16_t msgSize;         /* message size in msg */
static uint8_t msg[MAXMSGSIZE];  /* message body        */

uint8_t statusReg;  //

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

  // TODO: checksum error?
}

//uint8_t last = 0;         //
/* process message */
void processMessage() {
//  if (last != msg[0]) {   //
//    lcdWriteHex(msg[0]);  //
//    last = msg[0];        //
//  }                       //

  switch(msg[0]) {
    case CMD_SIGN_ON:
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
    case CMD_SET_PARAMETER:
      setParameter(&msgSize, msg);
      return;
      break;
    case CMD_GET_PARAMETER:
      getParameter(&msgSize, msg);
      return;
      break;
    case CMD_OSCCAL:
      if (msgSize == 1) {
        msgSize = 2;     // TODO
        msg[1] = STATUS_CMD_OK;
        statusReg = msg[1];
        return;
      }
      break;
    case CMD_LOAD_ADDRESS:
      loadAddress(&msgSize, msg);
      return;
    case CMD_FIRMWARE_UPGRADE:
      if (msgSize == 11) {
        msgSize = 2;
        msg[1] = STATUS_CMD_FAILED;     /* formware upgrade is always fail */
        statusReg = msg[1];
        return;
      }
    case CMD_ENTER_PROGMODE_ISP:
      enterProgModeIsp(&msgSize, msg);
      return;
      break;
    case CMD_LEAVE_PROGMODE_ISP:
      leaveProgModeIsp(&msgSize, msg);
      return;
      break;
    case CMD_CHIP_ERASE_ISP:
      chipEraseIsp(&msgSize, msg);
      return;
      break;
    case CMD_PROGRAM_FLASH_ISP:
      programFlashIsp(&msgSize, msg);
      return;
      break;

    case CMD_READ_FLASH_ISP:
      readFlashIsp(&msgSize, msg);
      return;
      break;

    case CMD_READ_FUSE_ISP:
    case CMD_READ_SIGNATURE_ISP:
      readFuseIsp(&msgSize, msg);
      return;
      break;
  }

  // TODO: error on unknown commands: STATUS_CMD_UNKNOWN

  lcdWriteHex(msg[0]);  ///
  // lcdWriteChr(' ');     ///
  msgSize = 2;                    /* unknown command or bad message */
  msg[1] = STATUS_CMD_FAILED;
  statusReg = msg[1];
  return;
}
