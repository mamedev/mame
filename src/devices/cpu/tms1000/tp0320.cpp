// license:BSD-3-Clause
// copyright-holders:hap
/*

  TMS1000 family - TP0320

*/

#include "tp0320.h"
#include "debugger.h"

// TP0320 is TI's first CMOS MCU with integrated LCD controller, the die is still very similar to TMS0980
// - 2048x9bit ROM, same as on TMS0980 with different row-select
// - 192x4bit RAM array at the bottom-left (set up as 16x12x4)
// - 16x4bit LCD RAM, above main RAM array
// - main instructions PLAs at the same position as TMS0980, fixed opcodes:
//   * LDP, RETN, OFF, bb?, be?, b9?, ba?, RBIT, SBIT, COMX8, bc?, LDX, XDA, TDO, SEAC, REAC, SAL, SBL
// - 64-term microinstructions PLA between the RAM and ROM, similar to TMS0980,
//   plus separate lines for custom opcode handling like TMS0270, used for SETR and RSTR
// - 24-term output PLA above LCD RAM
const device_type TP0320 = &device_creator<tp0320_cpu_device>; // 28-pin SDIP, ..


// internal memory maps
static ADDRESS_MAP_START(program_11bit_9, AS_PROGRAM, 16, tms1k_base_device)
	AM_RANGE(0x000, 0xfff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START(data_192x4, AS_DATA, 8, tms1k_base_device)
	AM_RANGE(0x00, 0x7f) AM_RAM
	AM_RANGE(0x80, 0xbf) AM_RAM AM_MIRROR(0x40) // DAM
ADDRESS_MAP_END


// device definitions
tp0320_cpu_device::tp0320_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: tms0980_cpu_device(mconfig, TP0320, "TP0320", tag, owner, clock, 7 /* o pins */, 10 /* r pins */, 7 /* pc bits */, 9 /* byte width */, 4 /* x width */, 12 /* prg width */, ADDRESS_MAP_NAME(program_11bit_9), 8 /* data width */, ADDRESS_MAP_NAME(data_192x4), "tp0320", __FILE__)
{ }


// disasm
offs_t tp0320_cpu_device::disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options)
{
	extern CPU_DISASSEMBLE(tp0320);
	return CPU_DISASSEMBLE_NAME(tp0320)(this, buffer, pc, oprom, opram, options);
}
