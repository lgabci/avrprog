/* AVR programmer prog */

#include "misc.h"
#include "prog.h"
#include "command.h"
#include "spi.h"
#include "lcd.h"   /////

static uint32_t address;

uint8_t enterProgModeIsp(uint8_t timeout, uint8_t stabDelay,
  uint8_t cmdexeDelay, uint8_t synchLoops, uint8_t byteDelay,
  uint8_t pollValue, uint8_t pollIndex, const uint8_t *cmd) {

  uint8_t ok;
  uint8_t i;
  uint8_t j;
  uint8_t rv;

  /*
  lcdWriteHex(timeout);     /// C8   200        Command timeout
  lcdWriteChr(' ');         ///
  lcdWriteHex(stabDelay);   /// 64   100    OK  Delay in ms for pin stabilization
  lcdWriteChr(' ');         ///
  lcdWriteHex(cmdexeDelay); /// 19    25    OK  Dealy in ms in connection with the EnterProgMode command execution
  lcdWriteChr(' ');         ///
  lcdWriteHex(synchLoops);  /// 20    32    OK  Number of syncronization loops
  lcdWriteChr(' ');         ///
  lcdWriteHex(byteDelay);   /// 00     0    OK  Delay in ms between each byte in the EnterProgModecommand
  lcdWriteChr(' ');         ///
  lcdWriteHex(pollValue);   /// 53          OK
  lcdWriteChr(' ');         ///
  lcdWriteHex(pollIndex);   /// 03     3    OK
  */


  /* SPI SCK = 0, SPI MOSI = 0, RESET = 0, this the default */
  delayMs(stabDelay);

  spiReset(1);  /* RESET: positive pulse, 2 CPU clock cycle */
  delayMs(2);   // 2* clock
  spiReset(0);

  delayMs(cmdexeDelay);

  ok = 0;
  for (j = 0; j < synchLoops; j ++) {

    // wdt

    for (i = 0; i < 4; i ++) {
      rv = spiTransmit(cmd[i]);  /* Programming Enable serial instruction  */
      delayMs(byteDelay);
      if (pollIndex == i + 1) {  /* don't stop on error, write all 4 bytes */
        ok = rv == pollValue;
      }
    }

    if (ok) {
      break;
    }

    spiSck(1);   /* trying to sync: SPI SCK pulse */
    delayMs(5);   // ? clock
    spiSck(0);
  }

  return ok ? STATUS_CMD_OK : STATUS_CMD_FAILED;
}


uint8_t readFuseIsp(uint8_t retAddr, const uint8_t *cmd, uint8_t *val) {
  uint8_t i;
  uint8_t v;
  uint8_t rv;

  v = 0;
  for (i = 0; i < 4; i ++) {
    rv = spiTransmit(cmd[i]);
    if (retAddr == i + 1) {
      v = rv;
    }
  }
  *val = v;

  return STATUS_CMD_OK;
}

uint8_t loadAddress(const uint8_t *cmd) {
//  lcdSetPos(0, 0);  //
//  lcdWriteHex(cmd[0]);  //
//  lcdWriteHex(cmd[1]);  //
//  lcdWriteHex(cmd[2]);  //
//  lcdWriteHex(cmd[3]);  //
//  lcdWriteChr(' ');  //

  address = (uint32_t)cmd[0] << 24 | (uint32_t)cmd[1] << 16 |
    (uint32_t)cmd[2] << 8 | cmd[3];

  return STATUS_CMD_OK;
}

uint8_t readFlashIsp(uint16_t blockSize, uint8_t cmd, uint8_t *data) {
//  lcdWriteChr('R');  //
//  lcdWriteHex(blockSize >> 8);  //
//  lcdWriteHex(blockSize & 0xff);  //
//  lcdWriteChr(' ');  //
//  lcdWriteHex(cmd);  //
//  lcdWriteChr(' ');  //

  while (blockSize --) {
    if (address & 0x80000000UL) {
      // load extended address
    }
    if (address & 1) {
      cmd |= _BV(3);
    }
    else {
      cmd &= ~ _BV(3);
    }
    spiTransmit(cmd);
    spiTransmit((uint8_t)(address >> 8));
    spiTransmit((uint8_t)address);
    address ++;
    *data ++ = spiTransmit(0);
//    lcdWriteHex(*(data - 1));  //
  }

  return STATUS_CMD_OK;
}
