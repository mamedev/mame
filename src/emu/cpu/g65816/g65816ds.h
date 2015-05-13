// license:BSD-3-Clause
// copyright-holders:Karl Stenerud
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

*/

unsigned g65816_disassemble(char* buff, unsigned int pc, unsigned int pb, const UINT8 *oprom, int m_flag, int x_flag);


#endif /* __G65816DS_H__ */
