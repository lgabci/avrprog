/* AVR programmer prog */

#include "prog.h"

unsigned char enterProgMode(unsigned char timeout, unsigned char stabDelay,
  unsigned char cmdexeDelay, unsigned char synchLoops, unsigned char byteDelay,
  unsigned char pollValue, unsigned char pollIndex, unsigned char cmd1,
  unsigned char cmd2, unsigned char cmd3, unsigned char cmd4) {
  const char h[16] = "0123456789ABCDEF"; /* // */
/* C8 64 19 20 00 53 03 AC 53 00 00
   stabDelay: after reset
   cmdexeDelay: wait before each loop
   synchLoops: synchronization loops, this many times try
*/

  while(1); /* // */
}
