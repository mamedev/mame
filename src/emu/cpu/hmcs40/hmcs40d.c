// license:BSD-3-Clause
// copyright-holders:hap
/*

  Hitachi HMCS40 MCU family disassembler

*/

#include "emu.h"
#include "debugger.h"
#include "hmcs40.h"


CPU_DISASSEMBLE(hmcs40)
{
	int pos = 2;//0;
//	UINT16 op = ((oprom[pos] << 8) | oprom[pos + 1]) & 0x3ff;
//	pos += 2;
//	UINT8 instr = hmcs40_mnemonic[op];

	char *dst = buffer;
	dst += sprintf(dst, "ABC");

	return pos | 0 | DASMFLAG_SUPPORTED;
}
