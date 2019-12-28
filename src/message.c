/* AVR programmer message */

#include "misc.h"
#include "message.h"
#include "serial.h"
#include "prog.h"
#include "command.h"

#define MAXMSGSIZE 100
static uint8_t seq;              /* message sequence    */
static uint16_t msgSize;         /* message size in msg */
static uint8_t chksum;           /* message checksum    */
static uint8_t msg[MAXMSGSIZE];  /* message body        */

uint8_t statusReg;  //

/* reads a message
   - return: 1 = succes, 0 = error */
void readMessage() {
  uint8_t c;
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

   get seq number   read char  store seq                 get msg size1

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
    if (! usartReceive(&c)) {
      continue;
    }
    if (c != MESSAGE_START) {
      continue;
    }
    chksum = c;

    /* get seq number */
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
    msgSize += c;

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
        break;
      }
      chksum ^= c;

      if (i < MAXMSGSIZE) {  /* store message if it is not too long */
        msg[i] = c;
      }
    }
    if (i < msgSize) {
      continue;
    }

    /* get checksum */
    if (! usartReceive(&c)) {
      continue;
    }
    chksum ^= c;

    return;
  }
}

/* send message */
void sendMessage() {
  uint16_t i;

/* parameter        size  description
   MSG_START           1  always 0x1B
   SEQ_NUMBER          1  incremented for each message, wraps after 0xFF
   MSG_SIZE            2  MSB first, size of message body
   TOKEN               1  always 0x0E
   MSG_BODY     MSG_SIZE  message body, from 0 to 65535 bytes
   CHECKSUM            1  XOR all bytes in message                         */

  usartTransmit(MESSAGE_START);
  chksum = MESSAGE_START;

  usartTransmit(seq);
  chksum ^= seq;

  usartTransmit(msgSize >> 8);
  chksum ^= msgSize >> 8;

  usartTransmit(msgSize & 0xff);
  chksum ^= msgSize & 0xff;

  usartTransmit(TOKEN);
  chksum ^= TOKEN;

  for (i = 0; i < msgSize; i ++) {
    usartTransmit(msg[i]);
    chksum ^= msg[i];
  }

  usartTransmit(chksum);
}

/* process message */
void processMessage() {
  if (chksum != 0) {    /* checksum error */
    cksumError(&msgSize, msg);
    return;
  }
  else {
    switch(msg[0]) {
      case CMD_SIGN_ON:
        signOn(&msgSize, msg);
        break;
      case CMD_SET_PARAMETER:
        setParameter(&msgSize, msg);
        break;
      case CMD_GET_PARAMETER:
        getParameter(&msgSize, msg);
        break;
      case CMD_OSCCAL:
        osccal(&msgSize, msg);
        break;
      case CMD_LOAD_ADDRESS:
        loadAddress(&msgSize, msg);
        break;
      case CMD_FIRMWARE_UPGRADE:
        firmwareUpgrade(&msgSize, msg);
        break;
      case CMD_ENTER_PROGMODE_ISP:
        enterProgModeIsp(&msgSize, msg);
        break;
      case CMD_LEAVE_PROGMODE_ISP:
        leaveProgModeIsp(&msgSize, msg);
        break;
      case CMD_CHIP_ERASE_ISP:
        chipEraseIsp(&msgSize, msg);
        break;
      case CMD_PROGRAM_FLASH_ISP:
        programFlashIsp(&msgSize, msg);
        break;
      case CMD_READ_FLASH_ISP:
        readFlashIsp(&msgSize, msg);
        break;
      case CMD_PROGRAM_EEPROM_ISP:
        programEepromIsp(&msgSize, msg);
        break;
      case CMD_READ_EEPROM_ISP:
        readEepromIsp(&msgSize, msg);
        break;
      case CMD_PROGRAM_FUSE_ISP:
        programFuseIsp(&msgSize, msg);
        break;
      case CMD_READ_FUSE_ISP:
        readFuseIsp(&msgSize, msg);
        break;
      case CMD_PROGRAM_LOCK_ISP:
        programLockIsp(&msgSize, msg);
        break;
      case CMD_READ_LOCK_ISP:
        readLockIsp(&msgSize, msg);
        break;
      case CMD_READ_SIGNATURE_ISP:
        readSignatureIsp(&msgSize, msg);
        break;
      case CMD_READ_OSCCAL_ISP:
        readOsccalIsp(&msgSize, msg);
        break;
      case CMD_SPI_MULTI:
        ispMulti(&msgSize, msg);
        break;
      default:
        msgSize = 2;                    /* unknown command or bad message */
        msg[1] = STATUS_CMD_UNKNOWN;
        statusReg = msg[1];
        break;
    }
  }
}
