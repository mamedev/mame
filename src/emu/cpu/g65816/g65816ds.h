#pragma once

#ifndef __G65816DS_H__
#define __G65816DS_H__
/* ======================================================================== */
/* =============================== COPYRIGHT ============================== */
/* ======================================================================== */
/*

G65C816 CPU Emulator V0.92

Copyright Karl Stenerud
All rights reserved.

Permission is granted to use this source code for non-commercial purposes.
To use this code for commercial purposes, you must get permission from the
author (Karl Stenerud) at karl@higashiyama-unet.ocn.ne.jp.


*/

unsigned g65816_disassemble(char* buff, unsigned int pc, unsigned int pb, const UINT8 *oprom, int m_flag, int x_flag);


#endif /* __G65816DS_H__ */
