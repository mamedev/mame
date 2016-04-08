// license:BSD-3-Clause
// copyright-holders:hap, Igor
/*

  Sharp SM500 MCU core implementation

*/

#include "sm500.h"
#include "debugger.h"


// MCU types
const device_type SM500 = &device_creator<sm500_device>;


// internal memory maps
static ADDRESS_MAP_START(program_2_7k, AS_PROGRAM, 8, sm510_base_device)
	AM_RANGE(0x0000, 0x00ff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START(data_96_32x4, AS_DATA, 8, sm510_base_device)
	AM_RANGE(0x00, 0x1f) AM_RAM
ADDRESS_MAP_END


// device definitions
sm500_device::sm500_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: sm510_base_device(mconfig, SM500, "SM500", tag, owner, clock, 2 /* stack levels */, 12 /* prg width */, ADDRESS_MAP_NAME(program_2_7k), 7 /* data width */, ADDRESS_MAP_NAME(data_96_32x4), "sm500", __FILE__)
{ }


// disasm
offs_t sm500_device::disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options)
{
	extern CPU_DISASSEMBLE(sm500);
	return CPU_DISASSEMBLE_NAME(sm500)(this, buffer, pc, oprom, opram, options);
}
