// license:BSD-3-Clause
// copyright-holders:hap
/*

  Mitsubishi M58846 MCU

  TODO:
  - o hai

*/

#include "m58846.h"
#include "debugger.h"



const device_type M58846 = &device_creator<m58846_device>;


// internal memory maps
static ADDRESS_MAP_START(program_1k, AS_PROGRAM, 16, melps4_cpu_device)
	AM_RANGE(0x0000, 0x03ff) AM_ROM
ADDRESS_MAP_END


static ADDRESS_MAP_START(data_64x4, AS_DATA, 8, melps4_cpu_device)
	AM_RANGE(0x00, 0x3f) AM_RAM
ADDRESS_MAP_END


// device definitions
m58846_device::m58846_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: melps4_cpu_device(mconfig, M58846, "M58846", tag, owner, clock, 10, ADDRESS_MAP_NAME(program_1k), 6, ADDRESS_MAP_NAME(data_64x4), "m58846", __FILE__)
{ }


// disasm
offs_t m58846_device::disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options)
{
	extern CPU_DISASSEMBLE(m58846);
	return CPU_DISASSEMBLE_NAME(m58846)(this, buffer, pc, oprom, opram, options);
}


