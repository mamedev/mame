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


// device definitions
tms1100_cpu_device::tms1100_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	tms1100_cpu_device(mconfig, TMS1100, tag, owner, clock, 8 /* o pins */, 11 /* r pins */, 6 /* pc bits */, 8 /* byte width */, 3 /* x width */, 1 /* stack levels */, 11 /* rom width */, address_map_constructor(FUNC(tms1100_cpu_device::rom_11bit), this), 7 /* ram width */, address_map_constructor(FUNC(tms1100_cpu_device::ram_7bit), this))
{ }

tms1100_cpu_device::tms1100_cpu_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, u8 o_pins, u8 r_pins, u8 pc_bits, u8 byte_bits, u8 x_bits, u8 stack_levels, int rom_width, address_map_constructor rom_map, int ram_width, address_map_constructor ram_map) :
	tms1000_cpu_device(mconfig, type, tag, owner, clock, o_pins, r_pins, pc_bits, byte_bits, x_bits, stack_levels, rom_width, rom_map, ram_width, ram_map)
{ }

tms1170_cpu_device::tms1170_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	tms1100_cpu_device(mconfig, TMS1170, tag, owner, clock, 8, 11, 6, 8, 3, 1, 11, address_map_constructor(FUNC(tms1170_cpu_device::rom_11bit), this), 7, address_map_constructor(FUNC(tms1170_cpu_device::ram_7bit), this))
{ }

tms1300_cpu_device::tms1300_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	tms1100_cpu_device(mconfig, TMS1300, tag, owner, clock, 8, 16, 6, 8, 3, 1, 11, address_map_constructor(FUNC(tms1300_cpu_device::rom_11bit), this), 7, address_map_constructor(FUNC(tms1300_cpu_device::ram_7bit), this))
{ }

tms1370_cpu_device::tms1370_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	tms1100_cpu_device(mconfig, TMS1370, tag, owner, clock, 8, 16, 6, 8, 3, 1, 11, address_map_constructor(FUNC(tms1370_cpu_device::rom_11bit), this), 7, address_map_constructor(FUNC(tms1370_cpu_device::ram_7bit), this))
{ }


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
	// SETR: supports 5-bit index with X MSB (used when it has more than 16 R pins)
	// TMS1100 manual simply says that X must be less than 4
	u8 index = BIT(m_x, m_x_bits - 1) << 4 | m_y;
	m_r = m_r | (1 << index);
	m_write_r(m_r & m_r_mask);
}

void tms1100_cpu_device::op_rstr()
{
	// RSTR: see SETR
	u8 index = BIT(m_x, m_x_bits - 1) << 4 | m_y;
	m_r = m_r & ~(1 << index);
	m_write_r(m_r & m_r_mask);
}
