#ifndef HEADER__SPC700DS
#define HEADER__SPC700DS
/* ======================================================================== */
/* =============================== COPYRIGHT ============================== */
/* ======================================================================== */
/*

Sony SPC700 CPU Emulator V1.0

Copyright Karl Stenerud
All rights reserved.

Permission is granted to use this source code for non-commercial purposes.
To use this code for commercial purposes, you must get permission from the
author (Karl Stenerud) at karl@higashiyama-unet.ocn.ne.jp.


*/

#include "cpuintrf.h"

offs_t spc700_dasm(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram);

#define spc700_read_8_disassembler(addr)				program_read_byte_8(addr)


#endif /* HEADER__SPC700DS */
