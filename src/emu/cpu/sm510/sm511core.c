// license:BSD-3-Clause
// copyright-holders:hap
/*

  Sharp SM511 MCU core implementation

*/

#include "sm510.h"
#include "debugger.h"


// MCU types
const device_type SM511 = &device_creator<sm511_device>;
const device_type SM512 = &device_creator<sm512_device>;


// internal memory maps
static ADDRESS_MAP_START(program_4k, AS_PROGRAM, 8, sm510_base_device)
	AM_RANGE(0x0000, 0x0fff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START(data_96_32x4, AS_DATA, 8, sm510_base_device)
	AM_RANGE(0x00, 0x5f) AM_RAM
	AM_RANGE(0x60, 0x7f) AM_RAM AM_SHARE("lcd_ram")
ADDRESS_MAP_END

static ADDRESS_MAP_START(data_80_48x4, AS_DATA, 8, sm510_base_device)
	AM_RANGE(0x00, 0x4f) AM_RAM
	AM_RANGE(0x50, 0x7f) AM_RAM AM_SHARE("lcd_ram")
ADDRESS_MAP_END


// disasm
offs_t sm511_device::disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options)
{
	extern CPU_DISASSEMBLE(sm511);
	return CPU_DISASSEMBLE_NAME(sm511)(this, buffer, pc, oprom, opram, options);
}


// device definitions
sm511_device::sm511_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: sm510_base_device(mconfig, SM511, "SM511", tag, owner, clock, 2 /* stack levels */, 12 /* prg width */, ADDRESS_MAP_NAME(program_4k), 7 /* data width */, ADDRESS_MAP_NAME(data_96_32x4), "sm511", __FILE__)
{ }

sm511_device::sm511_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, int stack_levels, int prgwidth, address_map_constructor program, int datawidth, address_map_constructor data, const char *shortname, const char *source)
	: sm510_base_device(mconfig, type, name, tag, owner, clock, stack_levels, prgwidth, program, datawidth, data, shortname, source)
{ }

sm512_device::sm512_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: sm511_device(mconfig, SM512, "SM512", tag, owner, clock, 2, 12, ADDRESS_MAP_NAME(program_4k), 7, ADDRESS_MAP_NAME(data_80_48x4), "sm512", __FILE__)
{ }



//-------------------------------------------------
//  execute
//-------------------------------------------------

void sm511_device::get_opcode_param()
{
}

void sm511_device::execute_one()
{
}
