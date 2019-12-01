/* AVR programmer prog header */

#ifndef __prog_h__
#define __prog_h__

#include "misc.h"

uint8_t enterProgModeIsp(uint8_t timeout, uint8_t stabDelay,
  uint8_t cmdexeDelay, uint8_t synchLoops, uint8_t byteDelay,
  uint8_t pollValue, uint8_t pollIndex, const uint8_t *cmd);
uint8_t readFuseIsp(uint8_t retAddr, const uint8_t *cmd, uint8_t *val);
uint8_t loadAddress(const uint8_t *cmd);
uint8_t readFlashIsp(uint16_t blockSize, uint8_t cmd, uint8_t *data);

#endif
