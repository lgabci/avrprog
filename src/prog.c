/* AVR programmer prog */

#include "misc.h"
#include "prog.h"
#include "command.h"
#include "spi.h"
#include "led.h"

#define BUILD_NUMBER_LOW    0
#define BUILD_NUMBER_HIGH   1
#define HW_VER              0
#define SW_MAJOR            0
#define SW_MINOR            1

static uint8_t statusReg = STATUS_CMD_OK;  /* status register              */

static uint32_t address = 0;
static uint8_t vtarget = 50;       /* VTARGET, always 5.0v                 */
static uint8_t vadjust = 50;       /* VADJUST, always 5.0v                 */
static uint8_t resetPolarity = 0;  /* 0 = AT89 (8051), 1 = AT90 (AVR)      */
static uint8_t oscPscale = 0;      /* oscillator timer prescaler value     */
static uint8_t oscCmatch = 0;      /* oscillator timer compare match value */
static uint8_t sckDuration = 0;    /* ISP clock duration                   */
static uint8_t controllerInit = 0; /* Controller init value = 0            */

#define RESET(v)   (((v) & 1) ^ resetPolarity)

/* high bit for addressing bytes */
#define HBIT(a) ((a) & 1 ? _BV(3) : 0)

/* mode bits */
#define MODEPAGE     _BV(0)       /* page mode */
#define MODEWTIMED   _BV(1)       /* word mode, timed delay */
#define MODEWVPOLL   _BV(2)       /* word mode, value polling */
#define MODEWRBPOLL  _BV(3)       /* word mode, RDY/BSY polling */
#define MODEPTIMED   _BV(4)       /* page mode, timed delay */
#define MODEPVPOLL   _BV(5)       /* page mode, value polling */
#define MODEPRBPOLL  _BV(6)       /* page mode, RDY/BSY polling */
#define MODEPWTRPG   _BV(7)       /* page mode, write page */

#define MEM_FLASH    0
#define MEM_EEPROM   1

static void setLeds() {
  if (statusReg == STATUS_CMD_OK) {
    ledOk();
  }
  else {
    ledError();
  }
}

/* send a 4 byte package on SPI bus
   - c: bytes to send
   - n: return this byte
   returns the nth byte                   */
static uint8_t transmitPacket(uint8_t c[4], uint8_t n) {
  uint8_t i;
  uint8_t v;
  uint8_t ret;

  ret = 0;
  ledSetTx();
  for (i = 0; i < 4; i ++) {
    v = spiTransmit(c[i]);
    if (n == i) {
      ret = v;
    }
  }
  ledClrTx();

  return ret;
}

static void readMemoryIsp(uint16_t *msgSize, uint8_t *msg, uint8_t memType) {  // TODO: memType enum
  uint16_t blockSize = (uint16_t)msg[1] << 8 | msg[2];
  uint8_t cmd = msg[3];
  uint8_t *data = &msg[2];
  uint8_t *status1 = &msg[1];
  uint8_t *status2 = &msg[blockSize + 2];

  uint16_t i;

  ledSetProcessing();

  if (*msgSize == 4) {
    if (memType == MEM_FLASH && address & 0x80000000UL) {
      // TODO: load extended address
    }

    for (i = 0; i < blockSize; i ++) {
      data[i] = transmitPacket(
        (uint8_t[4]){cmd | (memType == MEM_FLASH ? HBIT(i) : 0),
          (uint8_t)(address >> 8),
          (uint8_t)(address & 0xff),
          0
        }, 3);

      if (memType == MEM_EEPROM || HBIT(i)) {
        address ++;
      }
    }

    *msgSize = blockSize + 3;
    statusReg = STATUS_CMD_OK;
    *status2 = statusReg;
  }
  else {
    *msgSize = 2;
    statusReg = STATUS_CMD_FAILED;
  }
  *status1 = statusReg;

  setLeds();
}

static void programMemoryIsp(uint16_t *msgSize, uint8_t *msg, uint8_t memType) {  // TODO: memType enum
  uint16_t numOfBytes = ((uint16_t)msg[1] << 8 | msg[2]);
  uint8_t mode = msg[3];
  uint8_t delay = msg[4];
  uint8_t *cmd = &msg[5];
  uint8_t pollValue1 = msg[8];
  uint8_t pollValue2 = msg[9];
  uint8_t *data = &msg[10];
  uint8_t *status = &msg[1];

  uint16_t i;
  uint8_t rv;
  uint16_t oAddress;       /* original address */

  ledSetProcessing();

  if (*msgSize >= 10 && *msgSize == numOfBytes + 10) {
    if (memType == MEM_FLASH && address & 0x80000000UL) {
      // load extended address
    }

    oAddress = (uint16_t)address;
    for (i = 0, rv = STATUS_CMD_OK; i < numOfBytes && rv == STATUS_CMD_OK;
      memType == MEM_EEPROM || HBIT(i) ? address ++ : 0, i ++) {

      // wdt_reset

      /* 1st. command:
         - word mode: Write Program Memory
         - page mode: Load Program Memory page   */
      transmitPacket(
        (uint8_t[4]){cmd[0] | (memType == MEM_FLASH ? HBIT(i) : 0),
          (uint8_t)(address >> 8),
          (uint8_t)(address & 0xff),
          data[i]
        }, 0);

      if (mode & MODEPAGE) {      /* page mode */
        if (i < numOfBytes - 1 || ! (mode & MODEPWTRPG)) {
          continue;
        }

        /* 2nd command: Write Program Memory Page
           only in page mode, when a page finished and MODEPWTRPG is set */
        transmitPacket(
          (uint8_t[4]){cmd[1],
            (uint8_t)(oAddress >> 8),
            (uint8_t)(oAddress & 0xff),
            0
          }, 0);
      }

      /* waiting for MCU to be ready after each word or page */
      if ((! (mode & MODEPAGE) && mode & MODEWTIMED) ||  /* word RDY/BSY poll */
        (mode & MODEPAGE && mode & MODEPTIMED)) {     /* page RDY/BSY polling */
        delayMs(delay);
      }
      else if ((! (mode & MODEPAGE) && mode & MODEWVPOLL) || /* word val poll */
        (mode & MODEPAGE && mode & MODEPVPOLL)) {       /* page value polling */
        if (data[i] == pollValue1 ||   /* can not poll, have to wait */
          (memType == MEM_EEPROM && data[i] == pollValue2)) {
          delayMs(delay);
        }
        else {
          // set timeout: delay

          while (1) {
            /* 3rd command: Read Program Memory */
            if (transmitPacket(
              (uint8_t[4]){cmd[2] | (memType == MEM_FLASH ? HBIT(i) : 0),
                (uint8_t)(address >> 8),
                (uint8_t)(address & 0xff),
                0
              }, 3) == data[i]) {
              break;
            }
          }
        }
      }
      else if ((! (mode & MODEPAGE) && mode & MODEWRBPOLL) ||  /* w RDY/BSY p */
        (mode & MODEPAGE && mode & MODEPRBPOLL)) {    /* page RDY/BSY polling */
        // set timeout: delay

        while (1) {  // TODO: to func
          if (! transmitPacket((uint8_t[4]){0x0f, 0, 0, 0}, 3) & 1) {  /* poll RDY/BSY */   // TODO: make #define for 0x0f
            break;
          }

          // if (timeout) {
          //   rv = STATUS_CMD_RDY_BSY_TOUT;
          //   break;
          // }
        }
      }
    }

    *msgSize = 2;
    statusReg = rv;
  }
  else {
    *msgSize = 2;
    statusReg = STATUS_CMD_FAILED;
  }

  *status = statusReg;

  setLeds();
}

void signOn(uint16_t *msgSize, uint8_t *msg) {
  uint8_t *status = &msg[1];
  uint8_t *len = &msg[2];
  uint8_t *sign = &msg[3];
  const uint8_t *stk = (uint8_t *)"STK500_2";

  uint8_t i;

  ledSetProcessing();

  if (*msgSize == 1) {
    *msgSize = 11;
    *len = 8;                   /* length of signature string */
    for(i = *len; i; i --) {
      *(sign ++) = *(stk ++);
    }
    statusReg = STATUS_CMD_OK;
  }
  else {
    *msgSize = 2;
    statusReg = STATUS_CMD_FAILED;
  }

  *status = statusReg;

  setLeds();
}

void setParameter(uint16_t *msgSize, uint8_t *msg) {
  uint8_t par = msg[1];
  uint8_t val = msg[2];
  uint8_t *status = &msg[1];

  ledSetProcessing();

  if (*msgSize == 3) {
    switch (par) {
      case PARAM_VTARGET:
        if (val == 50) {  /* accpet only 5.0V */
          vtarget = val;
          statusReg = STATUS_CMD_OK;
        }
        else {
          statusReg = STATUS_CMD_FAILED;
        }
        break;
      case PARAM_VADJUST:
        if (val == 50) {  /* accpet only 5.0V */
          vadjust = val;
          statusReg = STATUS_CMD_OK;
        }
        else {
          statusReg = STATUS_CMD_FAILED;
        }
        break;
      case PARAM_OSC_PSCALE:
        if (val <= 0x07) {
          oscPscale = val;  // TODO
          statusReg = STATUS_CMD_OK;
        }
        else {
          statusReg = STATUS_CMD_FAILED;
        }
        break;
      case PARAM_OSC_CMATCH:
        oscCmatch = val;  // TODO
        statusReg = STATUS_CMD_OK;
        break;
      case PARAM_SCK_DURATION:
        sckDuration = val;
        spiSetSckDuration(sckDuration);
        statusReg = STATUS_CMD_OK;
        break;
      case PARAM_RESET_POLARITY:
        switch (val) {
          case 0:       /* AT89 (8051): active high reset */
          case 1:       /* AT90 (AVR): active low reset */
            resetPolarity = val;
            statusReg = STATUS_CMD_OK;
            break;
          default:
            statusReg = STATUS_CMD_FAILED;
            break;
        }
      case PARAM_CONTROLLER_INIT:
        controllerInit = val;
        statusReg = STATUS_CMD_OK;
        break;
      default:          /* unknown or read only parameter */
        statusReg = STATUS_CMD_FAILED;
        break;
    }

    *msgSize = 2;
  }
  else {
    *msgSize = 2;
    statusReg = STATUS_CMD_FAILED;
  }

  *status = statusReg;

  setLeds();
}

void getParameter(uint16_t *msgSize, uint8_t *msg) {
  uint8_t par = msg[1];
  uint8_t *val = &msg[2];
  uint8_t *status = &msg[1];

  ledSetProcessing();

  // TODO: STATUS_SET_PARAM_MISSING

  if (*msgSize == 2) {
    switch(par) {
      case PARAM_BUILD_NUMBER_LOW:
        *val = BUILD_NUMBER_LOW;
        statusReg = STATUS_CMD_OK;
        break;
      case PARAM_BUILD_NUMBER_HIGH:
        *val = BUILD_NUMBER_HIGH;
        statusReg = STATUS_CMD_OK;
        break;
      case PARAM_HW_VER:
        *val = HW_VER;
        statusReg = STATUS_CMD_OK;
        break;
      case PARAM_SW_MAJOR:
        *val = SW_MAJOR;
        statusReg = STATUS_CMD_OK;
        break;
      case PARAM_SW_MINOR:
        *val = SW_MINOR;
        statusReg = STATUS_CMD_OK;
        break;
      case PARAM_VTARGET:
        *val = vtarget;
        statusReg = STATUS_CMD_OK;
        break;
      case PARAM_VADJUST:
        *val = vadjust;
        statusReg = STATUS_CMD_OK;
        break;
      case PARAM_OSC_PSCALE:
        *val = oscPscale;
        statusReg = STATUS_CMD_OK;
        break;
      case PARAM_OSC_CMATCH:
        *val = oscCmatch;
        statusReg = STATUS_CMD_OK;
        break;
      case PARAM_SCK_DURATION:
        *val = sckDuration;
        statusReg = STATUS_CMD_OK;
        break;
      case PARAM_TOPCARD_DETECT:
        *val = 0;    /* no topcard found */
        statusReg = STATUS_CMD_OK;
        break;
      case PARAM_STATUS:
        *val = statusReg;
        statusReg = STATUS_CMD_OK;
        break;
      case PARAM_DATA:
        *val = 0;
        statusReg = STATUS_CMD_FAILED;  /* used only in high volt programming mode */
        break;
      case PARAM_RESET_POLARITY:
        *val = resetPolarity;
        statusReg = STATUS_CMD_OK;
        break;
      case PARAM_CONTROLLER_INIT:
        *val = controllerInit;
        statusReg = STATUS_CMD_OK;
        break;
      default:                      /* unknown parameter */
        statusReg = STATUS_CMD_FAILED;
        break;
    }

    *msgSize = statusReg == STATUS_CMD_OK ? 3 : 2;
  }
  else {
    *msgSize = 2;
    statusReg = STATUS_CMD_FAILED;
  }

  *status = statusReg;

  setLeds();
}

void osccal(uint16_t *msgSize, uint8_t *msg) {
  uint8_t *status = &msg[1];

  ledSetProcessing();

  if (*msgSize == 1) {
    *msgSize = 2;      /* not implemented */
    statusReg = STATUS_CMD_FAILED;
  }
  else {
    *msgSize = 2;
    statusReg = STATUS_CMD_FAILED;
  }
  *status = statusReg;

  setLeds();
}

void loadAddress(uint16_t *msgSize, uint8_t *msg) {
  uint8_t *cmd = &msg[1];
  uint8_t *status = &msg[1];

  ledSetProcessing();

  if (*msgSize == 5) {
    address = (uint32_t)cmd[0] << 24 | (uint32_t)cmd[1] << 16 |
      (uint32_t)cmd[2] << 8 | cmd[3];

    *msgSize = 2;
    statusReg = STATUS_CMD_OK;
  }
  else {
    *msgSize = 2;
    statusReg = STATUS_CMD_FAILED;
  }
  *status = statusReg;

  setLeds();
}

void firmwareUpgrade(uint16_t *msgSize, uint8_t *msg) {
  uint8_t *status = &msg[1];

  ledSetProcessing();

  if (*msgSize == 11) {
    *msgSize = 2;
    statusReg = STATUS_CMD_FAILED;  /* firmware upgrade always fails */
  }
  else {
    *msgSize = 2;
    statusReg = STATUS_CMD_FAILED;  // TODO: maybe other error?
  }
  *status = statusReg;

  setLeds();
}

void enterProgModeIsp(uint16_t *msgSize, uint8_t *msg) {
  //uint8_t timeout = msg[1];
  uint8_t stabDelay = msg[2];
  uint8_t cmdexeDelay = msg[3];
  uint8_t synchLoops = msg[4];
  uint8_t byteDelay = msg[5];
  uint8_t pollValue = msg[6];
  uint8_t pollIndex = msg[7];
  uint8_t *cmd = &msg[8];
  uint8_t *status = &msg[1];

  uint8_t ok = 0;   // ?
  uint8_t i;
  uint8_t j;
  uint8_t v;

  ledSetProcessing();

  if (*msgSize == 12) {
    spiInit();           /* SPI SCK = 0, SPI MOSI = 0, RESET = 0 */
    spiReset(RESET(1));
    delayMs(stabDelay);

    spiReset(RESET(0));  /* RESET  pulse */
    spiClockDelay();
    spiReset(RESET(1));

    delayMs(cmdexeDelay);

    ok = 0;
    for (i = 0; i < synchLoops; i ++) {

      // wdt reset

      ledSetTx();
      for (j = 0; j < 4; j ++) {
        v = spiTransmit(cmd[j]);
        delayMs(byteDelay);

        if (j == pollIndex - 1 && v == pollValue) {
          ok = 1;
        }
      }
      ledClrTx();

      if (ok) {
        break;
      }

      spiSck(1);   /* trying to sync: SPI SCK pulse */
      spiClockDelay();
      spiSck(0);
      spiClockDelay();
    }

    *msgSize = 2;
    statusReg = ok ? STATUS_CMD_OK : STATUS_CMD_FAILED;
  }
  else {
    *msgSize = 2;
    statusReg = STATUS_CMD_FAILED;
  }
  *status = statusReg;

  setLeds();
}

void leaveProgModeIsp(uint16_t *msgSize, uint8_t *msg) {
  uint8_t preDelay = msg[1];
  uint8_t postDelay = msg[2];
  uint8_t *status = &msg[1];

  uint8_t sr;  /* use local status, don't set statusReg */

  ledSetProcessing();

  if (*msgSize == 3) {
    spiReset(RESET(1));
    spiMosi(0);
    spiSck(0);
    delayMs(preDelay);
    spiClose();
    delayMs(postDelay);

    *msgSize = 2;
    sr = STATUS_CMD_OK;
  }
  else {
    *msgSize = 2;
    sr = STATUS_CMD_FAILED;
  }
  *status = sr;

  setLeds();
}

void chipEraseIsp(uint16_t *msgSize, uint8_t *msg) {
  uint8_t eraseDelay = msg[1];
  uint8_t pollMethod = msg[2];
  uint8_t *cmd = &msg[3];
  uint8_t *status = &msg[1];

  uint8_t rv;

  ledSetProcessing();

  if (*msgSize == 7) {
    rv = STATUS_CMD_OK;
    transmitPacket(cmd, 0);
    if (pollMethod) {
      // set timeout: delay

      while (1) {   // to func
        if (! transmitPacket((uint8_t[4]){0x0f, 0, 0, 0}, 3) & 1) {  // TODO: make #define for 0x0f
          break;
        }

        // if (timeout) {
        //   rv = STATUS_CMD_RDY_BSY_TOUT;
        //   break;
        // }
      }
    }
    else {
      delayMs(eraseDelay);
    }

    *msgSize = 2;
    statusReg = rv;
  }
  else {
    *msgSize = 2;
    statusReg = STATUS_CMD_FAILED;
  }

  *status = statusReg;

  setLeds();
}

void programFlashIsp(uint16_t *msgSize, uint8_t *msg) {
  programMemoryIsp(msgSize, msg, MEM_FLASH);
}

void readFlashIsp(uint16_t *msgSize, uint8_t *msg) {
  readMemoryIsp(msgSize, msg, MEM_FLASH);
}

void programEepromIsp(uint16_t *msgSize, uint8_t *msg) {
  programMemoryIsp(msgSize, msg, MEM_EEPROM);
}

void readEepromIsp(uint16_t *msgSize, uint8_t *msg) {
  readMemoryIsp(msgSize, msg, MEM_EEPROM);
}

void programFuseIsp(uint16_t *msgSize, uint8_t *msg) {
  uint8_t *status1 = &msg[1];
  uint8_t *status2 = &msg[2];
  uint8_t *cmd = &msg[1];

  ledSetProcessing();

  if (*msgSize == 5) {
    transmitPacket(cmd, 0);

    *msgSize = 3;
    statusReg = STATUS_CMD_OK;
    *status2 = statusReg;
  }
  else {
    *msgSize = 2;
    statusReg = STATUS_CMD_FAILED;
  }
  *status1 = statusReg;

  setLeds();
}

void readFuseIsp(uint16_t *msgSize, uint8_t *msg) {
  uint8_t retAddr = msg[1];
  uint8_t *cmd = &msg[2];
  uint8_t *val = &msg[2];
  uint8_t *status1 = &msg[1];
  uint8_t *status2 = &msg[3];

  ledSetProcessing();

  if (*msgSize == 6) {
    *val = transmitPacket(cmd, retAddr - 1);

    *msgSize = 4;
    statusReg = STATUS_CMD_OK;
    *status2 = statusReg;
  }
  else {
    *msgSize = 2;
    statusReg = STATUS_CMD_FAILED;
  }
  *status1 = statusReg;

  setLeds();
}

void programLockIsp(uint16_t *msgSize, uint8_t *msg) {
  programFuseIsp(msgSize, msg);
}

void readLockIsp(uint16_t *msgSize, uint8_t *msg) {
  readFuseIsp(msgSize, msg);
}

void readSignatureIsp(uint16_t *msgSize, uint8_t *msg) {
  readFuseIsp(msgSize, msg);
}

void readOsccalIsp(uint16_t *msgSize, uint8_t *msg) {
  readFuseIsp(msgSize, msg);
}

void ispMulti(uint16_t *msgSize, uint8_t *msg) {
  uint8_t numTx = msg[1];
  uint8_t numRx = msg[2];
  uint8_t rxStartAddr = msg[3];
  uint8_t *txData = &msg[4];

  uint8_t *status1 = &msg[1];
  uint8_t *data = &msg[2];
  uint8_t *status2 = &msg[numRx + 2];

  ledSetProcessing();


  // TODO: wdt reset?

  if (*msgSize >= 3 && *msgSize == numRx + 4) {
    uint8_t v;

    *msgSize = numRx + 3;
    ledSetTx();
    while (numTx || numRx) {
      if (numTx) {
        v = spiTransmit(*(txData ++));
        numTx --;
      }
      else {
        v = spiTransmit(0);
      }

      if (rxStartAddr == 0) {
        if (numRx) {
          *(data ++) = v;
          numRx --;
        }
      }
      else {
        rxStartAddr --;
      }
    }
    ledClrTx();

    statusReg = STATUS_CMD_OK;
    *status2 = statusReg;
  }
  else {
    *msgSize = 2;
    statusReg = STATUS_CMD_FAILED;
  }
  *status1 = statusReg;

  setLeds();
}

void cksumError(uint16_t *msgSize, uint8_t *msg) {
  uint8_t *cmd = &msg[0];
  uint8_t *status = &msg[1];

  ledSetProcessing();

  *msgSize = 2;
  *cmd = ANSWER_CKSUM_ERROR;
  statusReg = ANSWER_CKSUM_ERROR;
  *status = statusReg;

  setLeds();
}
