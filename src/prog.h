/* AVR programmer prog header */

#ifndef __prog_h__
#define __prog_h__

#include "misc.h"

void signOn(uint16_t *msgSize, uint8_t *msg);
void setParameter(uint16_t *msgSize, uint8_t *msg);
void getParameter(uint16_t *msgSize, uint8_t *msg);
void osccal(uint16_t *msgSize, uint8_t *msg);  //
void loadAddress(uint16_t *msgSize, uint8_t *msg);
void firmwareUpgrade(uint16_t *msgSize, uint8_t *msg);
void enterProgModeIsp(uint16_t *msgSize, uint8_t *msg);
void leaveProgModeIsp(uint16_t *msgSize, uint8_t *msg);
void chipEraseIsp(uint16_t *msgSize, uint8_t *msg);
void programFlashIsp(uint16_t *msgSize, uint8_t *msg);
void readFlashIsp(uint16_t *msgSize, uint8_t *msg);
void programEepromIsp(uint16_t *msgSize, uint8_t *msg);
void readEepromIsp(uint16_t *msgSize, uint8_t *msg);
void programFuseIsp(uint16_t *msgSize, uint8_t *msg);
void readFuseIsp(uint16_t *msgSize, uint8_t *msg);
void programLockIsp(uint16_t *msgSize, uint8_t *msg);
void readLockIsp(uint16_t *msgSize, uint8_t *msg);
void readSignatureIsp(uint16_t *msgSize, uint8_t *msg);
void readOsccalIsp(uint16_t *msgSize, uint8_t *msg);
void ispMulti(uint16_t *msgSize, uint8_t *msg);

void cksumError(uint16_t *msgSize, uint8_t *msg);  //

#endif
