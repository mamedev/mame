#pragma once

#ifndef __SPC700DS_H__
#define __SPC700DS_H__
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

CPU_DISASSEMBLE( spc700 );

#define spc700_read_8_disassembler(addr)				program_read_byte_8(addr)


#endif /* __SPC700DS_H__ */
