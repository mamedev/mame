// license:BSD-3-Clause
// copyright-holders:hap
/*

  TMS1000 family - TMS1100, TMS1170, TMS1300, TMS1370

  TODO:
  - add TMS1100C when needed

*/

#include "emu.h"
#include "tms1100.h"
#include "tms1k_dasm.h"

// TMS1100 is nearly the same as TMS1000, some different opcodes, and with double the RAM and ROM
DEFINE_DEVICE_TYPE(TMS1100, tms1100_cpu_device, "tms1100", "Texas Instruments TMS1100") // 28-pin DIP, 11 R pins
DEFINE_DEVICE_TYPE(TMS1170, tms1170_cpu_device, "tms1170", "Texas Instruments TMS1170") // high voltage version
DEFINE_DEVICE_TYPE(TMS1300, tms1300_cpu_device, "tms1300", "Texas Instruments TMS1300") // 40-pin DIP, 16 R pins
DEFINE_DEVICE_TYPE(TMS1370, tms1370_cpu_device, "tms1370", "Texas Instruments TMS1370") // high voltage version, also seen in 28-pin package(some O/R pins unavailable)


// internal memory maps
void tms1100_cpu_device::program_11bit_8(address_map &map)
{
	map(0x000, 0x7ff).rom();
}

void tms1100_cpu_device::data_128x4(address_map &map)
{
	map(0x00, 0x7f).ram();
}


// device definitions
tms1100_cpu_device::tms1100_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: tms1100_cpu_device(mconfig, TMS1100, tag, owner, clock, 8 /* o pins */, 11 /* r pins */, 6 /* pc bits */, 8 /* byte width */, 3 /* x width */, 11 /* prg width */, address_map_constructor(FUNC(tms1100_cpu_device::program_11bit_8), this), 7 /* data width */, address_map_constructor(FUNC(tms1100_cpu_device::data_128x4), this))
{
}

tms1100_cpu_device::tms1100_cpu_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, u8 o_pins, u8 r_pins, u8 pc_bits, u8 byte_bits, u8 x_bits, int prgwidth, address_map_constructor program, int datawidth, address_map_constructor data)
	: tms1000_cpu_device(mconfig, type, tag, owner, clock, o_pins, r_pins, pc_bits, byte_bits, x_bits, prgwidth, program, datawidth, data)
{
}

tms1170_cpu_device::tms1170_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: tms1100_cpu_device(mconfig, TMS1170, tag, owner, clock, 8, 11, 6, 8, 3, 11, address_map_constructor(FUNC(tms1170_cpu_device::program_11bit_8), this), 7, address_map_constructor(FUNC(tms1170_cpu_device::data_128x4), this))
{
}

tms1300_cpu_device::tms1300_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: tms1100_cpu_device(mconfig, TMS1300, tag, owner, clock, 8, 16, 6, 8, 3, 11, address_map_constructor(FUNC(tms1300_cpu_device::program_11bit_8), this), 7, address_map_constructor(FUNC(tms1300_cpu_device::data_128x4), this))
{
}

tms1370_cpu_device::tms1370_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: tms1100_cpu_device(mconfig, TMS1370, tag, owner, clock, 8, 16, 6, 8, 3, 11, address_map_constructor(FUNC(tms1370_cpu_device::program_11bit_8), this), 7, address_map_constructor(FUNC(tms1370_cpu_device::data_128x4), this))
{
}


// disasm
std::unique_ptr<util::disasm_interface> tms1100_cpu_device::create_disassembler()
{
	return std::make_unique<tms1100_disassembler>();
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
