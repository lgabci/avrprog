/* AVR programmer prog header */

#ifndef __prog_h__
#define __prog_h__

unsigned char enterProgMode(unsigned char timeout, unsigned char stabDelay,
  unsigned char cmdexeDelay, unsigned char synchLoops, unsigned char byteDelay,
  unsigned char pollValue, unsigned char pollIndex, unsigned char cmd1,
  unsigned char cmd2, unsigned char cmd3, unsigned char cmd4);

#endif
