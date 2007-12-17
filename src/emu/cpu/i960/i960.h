#ifndef __I960_H
#define __I960_H

#include "cpuintrf.h"

enum {
  I960_PFP = 0,
  I960_SP  = 1,
  I960_RIP = 2,
  I960_FP  = 31,

  I960_R0 = 0,
  I960_R1 = 1,
  I960_R2 = 2,
  I960_R3 = 3,
  I960_R4 = 4,
  I960_R5 = 5,
  I960_R6 = 6,
  I960_R7 = 7,
  I960_R8 = 8,
  I960_R9 = 9,
  I960_R10 = 10,
  I960_R11 = 11,
  I960_R12 = 12,
  I960_R13 = 13,
  I960_R14 = 14,
  I960_R15 = 15,
  I960_G0 = 16,
  I960_G1 = 17,
  I960_G2 = 18,
  I960_G3 = 19,
  I960_G4 = 20,
  I960_G5 = 21,
  I960_G6 = 22,
  I960_G7 = 23,
  I960_G8 = 24,
  I960_G9 = 25,
  I960_G10 = 26,
  I960_G11 = 27,
  I960_G12 = 28,
  I960_G13 = 29,
  I960_G14 = 30,
  I960_G15 = 31,

  I960_SAT = 32,
  I960_PRCB = 33,
  I960_PC = 34,
  I960_AC = 35,
  I960_IP = 36,
  I960_PIP = 37
};

enum {
  I960_IRQ0 = 0,
  I960_IRQ1 = 1,
  I960_IRQ2 = 2,
  I960_IRQ3 = 3
};

void i960_get_info(UINT32 state, cpuinfo *info);
void i960_noburst(void);
void i960_stall(void);

#endif
