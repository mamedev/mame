// license:BSD-3-Clause
// copyright-holders:hap
/*

  TMS1000 family - TMS2400, TMS2470, TMS2600, TMS2670

  TODO:
  - x

*/

#include "emu.h"
#include "tms2400.h"
#include "tms1k_dasm.h"

// TMS2400 is a TMS2100 with twice more memory (kind of like TMS1400 is to TMS1100)
DEFINE_DEVICE_TYPE(TMS2400, tms2400_cpu_device, "tms2400", "Texas Instruments TMS2400") // 28-pin DIP, 7 R pins
DEFINE_DEVICE_TYPE(TMS2470, tms2470_cpu_device, "tms2470", "Texas Instruments TMS2470") // high voltage version, 1 R pin removed for Vpp
DEFINE_DEVICE_TYPE(TMS2600, tms2600_cpu_device, "tms2600", "Texas Instruments TMS2600") // 40-pin DIP, 15 R pins, J pins
DEFINE_DEVICE_TYPE(TMS2670, tms2670_cpu_device, "tms2670", "Texas Instruments TMS2670") // high voltage version, 1 R pin removed for Vpp


// device definitions
tms2400_cpu_device::tms2400_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	tms2400_cpu_device(mconfig, TMS2400, tag, owner, clock, 8 /* o pins */, 7 /* r pins */, 6 /* pc bits */, 8 /* byte width */, 4 /* x width */, 4 /* stack levels */, 12 /* rom width */, address_map_constructor(FUNC(tms2400_cpu_device::rom_12bit), this), 8 /* ram width */, address_map_constructor(FUNC(tms2400_cpu_device::ram_8bit), this))
{ }

tms2400_cpu_device::tms2400_cpu_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, u8 o_pins, u8 r_pins, u8 pc_bits, u8 byte_bits, u8 x_bits, u8 stack_levels, int rom_width, address_map_constructor rom_map, int ram_width, address_map_constructor ram_map) :
	tms2100_cpu_device(mconfig, type, tag, owner, clock, o_pins, r_pins, pc_bits, byte_bits, x_bits, stack_levels, rom_width, rom_map, ram_width, ram_map)
{ }

tms2470_cpu_device::tms2470_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	tms2400_cpu_device(mconfig, TMS2470, tag, owner, clock, 8, 6, 6, 8, 4, 4, 12, address_map_constructor(FUNC(tms2470_cpu_device::rom_12bit), this), 8, address_map_constructor(FUNC(tms2470_cpu_device::ram_8bit), this))
{ }

tms2600_cpu_device::tms2600_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	tms2600_cpu_device(mconfig, TMS2600, tag, owner, clock, 8, 15, 6, 8, 4, 4, 12, address_map_constructor(FUNC(tms2600_cpu_device::rom_12bit), this), 8, address_map_constructor(FUNC(tms2600_cpu_device::ram_8bit), this))
{ }

tms2600_cpu_device::tms2600_cpu_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, u8 o_pins, u8 r_pins, u8 pc_bits, u8 byte_bits, u8 x_bits, u8 stack_levels, int rom_width, address_map_constructor rom_map, int ram_width, address_map_constructor ram_map) :
	tms2400_cpu_device(mconfig, type, tag, owner, clock, o_pins, r_pins, pc_bits, byte_bits, x_bits, stack_levels, rom_width, rom_map, ram_width, ram_map)
{ }

tms2670_cpu_device::tms2670_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	tms2600_cpu_device(mconfig, TMS2670, tag, owner, clock, 8, 14, 6, 8, 4, 4, 12, address_map_constructor(FUNC(tms2670_cpu_device::rom_12bit), this), 8, address_map_constructor(FUNC(tms2670_cpu_device::ram_8bit), this))
{ }


// disasm
std::unique_ptr<util::disasm_interface> tms2400_cpu_device::create_disassembler()
{
	return std::make_unique<tms2400_disassembler>();
}


// device_reset
void tms2400_cpu_device::device_reset()
{
	tms2100_cpu_device::device_reset();

	// changed/added fixed instructions
	m_fixed_decode[0x0b] = F_TPC;
}


// opcode deviations
void tms2400_cpu_device::op_ldx()
{
	// LDX: value is still 3 bit even though X is 4 bit
	tms2100_cpu_device::op_ldx();
	m_x >>= 1;
}
