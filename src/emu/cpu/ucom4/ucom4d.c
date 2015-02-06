// license:BSD-3-Clause
// copyright-holders:hap
/*

  NEC uCOM-4 MCU family disassembler

*/

#include "emu.h"
#include "debugger.h"
#include "ucom4.h"


CPU_DISASSEMBLE(ucom4)
{
	int pos = 0;
//	UINT8 op = oprom[pos++];
//	UINT8 instr = ucom4_mnemonic[op];

	char *dst = buffer;
	dst += sprintf(dst, "ABC");

	return pos | 0 | DASMFLAG_SUPPORTED;
}
