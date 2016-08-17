// license:BSD-3-Clause
// copyright-holders:hap
/*

  TMS1000 family - TMS1100, TMS1170, TMS1300, TMS1370

*/

#include "tms1100.h"
#include "debugger.h"

// TMS1100 is nearly the same as TMS1000, some different opcodes, and with double the RAM and ROM
const device_type TMS1100 = &device_creator<tms1100_cpu_device>; // 28-pin DIP, 11 R pins
const device_type TMS1170 = &device_creator<tms1170_cpu_device>; // high voltage version
const device_type TMS1300 = &device_creator<tms1300_cpu_device>; // 40-pin DIP, 16 R pins
const device_type TMS1370 = &device_creator<tms1370_cpu_device>; // high voltage version


// internal memory maps
static ADDRESS_MAP_START(program_11bit_8, AS_PROGRAM, 8, tms1k_base_device)
	AM_RANGE(0x000, 0x7ff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START(data_128x4, AS_DATA, 8, tms1k_base_device)
	AM_RANGE(0x00, 0x7f) AM_RAM
ADDRESS_MAP_END


// device definitions
tms1100_cpu_device::tms1100_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: tms1000_cpu_device(mconfig, TMS1100, "TMS1100", tag, owner, clock, 8 /* o pins */, 11 /* r pins */, 6 /* pc bits */, 8 /* byte width */, 3 /* x width */, 11 /* prg width */, ADDRESS_MAP_NAME(program_11bit_8), 7 /* data width */, ADDRESS_MAP_NAME(data_128x4), "tms1100", __FILE__)
{ }

tms1100_cpu_device::tms1100_cpu_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, UINT8 o_pins, UINT8 r_pins, UINT8 pc_bits, UINT8 byte_bits, UINT8 x_bits, int prgwidth, address_map_constructor program, int datawidth, address_map_constructor data, const char *shortname, const char *source)
	: tms1000_cpu_device(mconfig, type, name, tag, owner, clock, o_pins, r_pins, pc_bits, byte_bits, x_bits, prgwidth, program, datawidth, data, shortname, source)
{ }

tms1170_cpu_device::tms1170_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: tms1100_cpu_device(mconfig, TMS1170, "TMS1170", tag, owner, clock, 8, 11, 6, 8, 3, 11, ADDRESS_MAP_NAME(program_11bit_8), 7, ADDRESS_MAP_NAME(data_128x4), "tms1170", __FILE__)
{ }

tms1300_cpu_device::tms1300_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: tms1100_cpu_device(mconfig, TMS1300, "TMS1300", tag, owner, clock, 8, 16, 6, 8, 3, 11, ADDRESS_MAP_NAME(program_11bit_8), 7, ADDRESS_MAP_NAME(data_128x4), "tms1300", __FILE__)
{ }

tms1370_cpu_device::tms1370_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: tms1100_cpu_device(mconfig, TMS1370, "TMS1370", tag, owner, clock, 8, 16, 6, 8, 3, 11, ADDRESS_MAP_NAME(program_11bit_8), 7, ADDRESS_MAP_NAME(data_128x4), "tms1370", __FILE__)
{ }


// disasm
offs_t tms1100_cpu_device::disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options)
{
	extern CPU_DISASSEMBLE(tms1100);
	return CPU_DISASSEMBLE_NAME(tms1100)(this, buffer, pc, oprom, opram, options);
}


// device_reset
void tms1100_cpu_device::device_reset()
{
	tms1000_cpu_device::device_reset();

	// small differences in 00-3f area
	m_fixed_decode[0x00] = 0;
	m_fixed_decode[0x09] = F_COMX8; // !
	m_fixed_decode[0x0b] = F_COMC;

	for (int i = 0x28; i < 0x30; i++) m_fixed_decode[i] = F_LDX;
	for (int i = 0x3c; i < 0x40; i++) m_fixed_decode[i] = 0;
}


// opcode deviations
void tms1100_cpu_device::op_setr()
{
	// SETR: same, but X register MSB must be clear
	if (~m_x & (1 << (m_x_bits-1)))
		tms1k_base_device::op_setr();
}

void tms1100_cpu_device::op_rstr()
{
	// RSTR: same, but X register MSB must be clear
	if (~m_x & (1 << (m_x_bits-1)))
		tms1k_base_device::op_rstr();
}
