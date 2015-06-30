// license:BSD-3-Clause
// copyright-holders:hap
/*

  Sharp SM510 MCU family disassembler

*/

#include "emu.h"
#include "debugger.h"
#include "sm510.h"




CPU_DISASSEMBLE(sm510)
{
	//int pos = 0;
	//UINT8 op = oprom[pos++];
	return 1 | DASMFLAG_SUPPORTED;

}
