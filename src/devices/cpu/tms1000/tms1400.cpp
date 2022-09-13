// license:BSD-3-Clause
// copyright-holders:hap
/*

  TMS1000 family - TMS1400, TMS1470, TMS1475, TMS1600, TMS1670

TMS1400 follows the TMS1100, it doubles the ROM size again (4 chapters of 16 pages), and adds a 3-level callstack
- rotate the view and mirror the OR-mask to get the proper layout of the mpla, the default is identical to TMS1100
- the opla size is increased from 20 to 32 terms

TMS1475 adds more R pins (indexed from X MSB)
- opla OR order appears to be flipped: O7-O0

TMS1600 adds more I/O to the TMS1400, input pins are doubled with added L1,2,4,8
- mpla and opla are the same as TMS1400

TODO:
- emulate TMS1600 L-pins

*/

#include "emu.h"
#include "tms1400.h"
#include "tms1k_dasm.h"


// device definitions
DEFINE_DEVICE_TYPE(TMS1400, tms1400_cpu_device, "tms1400", "Texas Instruments TMS1400") // 28-pin DIP, 11 R pins (TMS1400CR is same, but with TMS1100 pinout)
DEFINE_DEVICE_TYPE(TMS1470, tms1470_cpu_device, "tms1470", "Texas Instruments TMS1470") // high voltage version, 1 R pin removed for Vpp
DEFINE_DEVICE_TYPE(TMS1475, tms1475_cpu_device, "tms1475", "Texas Instruments TMS1475") // 40-pin DIP, 22 R pins

DEFINE_DEVICE_TYPE(TMS1600, tms1600_cpu_device, "tms1600", "Texas Instruments TMS1600") // 40-pin DIP, 16 R pins
DEFINE_DEVICE_TYPE(TMS1670, tms1670_cpu_device, "tms1670", "Texas Instruments TMS1670") // high voltage version


tms1400_cpu_device::tms1400_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	tms1400_cpu_device(mconfig, TMS1400, tag, owner, clock, 8 /* o pins */, 11 /* r pins */, 6 /* pc bits */, 8 /* byte width */, 3 /* x width */, 3 /* stack levels */, 12 /* rom width */, address_map_constructor(FUNC(tms1400_cpu_device::rom_12bit), this), 7 /* ram width */, address_map_constructor(FUNC(tms1400_cpu_device::ram_7bit), this))
{ }

tms1400_cpu_device::tms1400_cpu_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, u8 o_pins, u8 r_pins, u8 pc_bits, u8 byte_bits, u8 x_bits, u8 stack_levels, int rom_width, address_map_constructor rom_map, int ram_width, address_map_constructor ram_map) :
	tms1100_cpu_device(mconfig, type, tag, owner, clock, o_pins, r_pins, pc_bits, byte_bits, x_bits, stack_levels, rom_width, rom_map, ram_width, ram_map)
{ }


tms1470_cpu_device::tms1470_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	tms1470_cpu_device(mconfig, TMS1470, tag, owner, clock, 8, 10, 6, 8, 3, 3, 12, address_map_constructor(FUNC(tms1470_cpu_device::rom_12bit), this), 7, address_map_constructor(FUNC(tms1470_cpu_device::ram_7bit), this))
{ }

tms1470_cpu_device::tms1470_cpu_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, u8 o_pins, u8 r_pins, u8 pc_bits, u8 byte_bits, u8 x_bits, u8 stack_levels, int rom_width, address_map_constructor rom_map, int ram_width, address_map_constructor ram_map) :
	tms1400_cpu_device(mconfig, type, tag, owner, clock, o_pins, r_pins, pc_bits, byte_bits, x_bits, stack_levels, rom_width, rom_map, ram_width, ram_map)
{ }

tms1475_cpu_device::tms1475_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	tms1470_cpu_device(mconfig, TMS1475, tag, owner, clock, 8, 22, 6, 8, 3, 3, 12, address_map_constructor(FUNC(tms1475_cpu_device::rom_12bit), this), 7, address_map_constructor(FUNC(tms1475_cpu_device::ram_7bit), this))
{ }


tms1600_cpu_device::tms1600_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	tms1600_cpu_device(mconfig, TMS1600, tag, owner, clock, 8, 16, 6, 8, 3, 3, 12, address_map_constructor(FUNC(tms1600_cpu_device::rom_12bit), this), 7, address_map_constructor(FUNC(tms1600_cpu_device::ram_7bit), this))
{ }

tms1600_cpu_device::tms1600_cpu_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, u8 o_pins, u8 r_pins, u8 pc_bits, u8 byte_bits, u8 x_bits, u8 stack_levels, int rom_width, address_map_constructor rom_map, int ram_width, address_map_constructor ram_map) :
	tms1400_cpu_device(mconfig, type, tag, owner, clock, o_pins, r_pins, pc_bits, byte_bits, x_bits, stack_levels, rom_width, rom_map, ram_width, ram_map)
{ }

tms1670_cpu_device::tms1670_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	tms1600_cpu_device(mconfig, TMS1670, tag, owner, clock, 8, 16, 6, 8, 3, 3, 12, address_map_constructor(FUNC(tms1670_cpu_device::rom_12bit), this), 7, address_map_constructor(FUNC(tms1670_cpu_device::ram_7bit), this))
{ }


// machine configs
void tms1400_cpu_device::device_add_mconfig(machine_config &config)
{
	// microinstructions PLA, output PLA
	PLA(config, "mpla", 8, 16, 30).set_format(pla_device::FMT::BERKELEY);
	PLA(config, "opla", 5, 8, 32).set_format(pla_device::FMT::BERKELEY);
}


// disasm
std::unique_ptr<util::disasm_interface> tms1400_cpu_device::create_disassembler()
{
	return std::make_unique<tms1400_disassembler>();
}


// device_reset
void tms1400_cpu_device::device_reset()
{
	tms1100_cpu_device::device_reset();

	// small differences in 00-3f area
	m_fixed_decode[0x0b] = F_TPC;
}
