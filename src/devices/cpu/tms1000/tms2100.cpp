// license:BSD-3-Clause
// copyright-holders:hap
/*

  TMS1000 family - TMS2100, TMS2170, TMS2300, TMS2370

  TODO:
  - x

*/

#include "emu.h"
#include "tms2100.h"
#include "tms1k_dasm.h"

// TMS2100 is an enhanced version of TMS1100, adding interrupt, timer, A/D converter, and a 4-level callstack
// - the mpla has a similar layout as TMS1400, terms reduced to 26 (looks like it's optimized and not meant to be custom)
// - the opla is the same as TMS1400
DEFINE_DEVICE_TYPE(TMS2100, tms2100_cpu_device, "tms2100", "Texas Instruments TMS2100") // 28-pin DIP, 7 R pins
DEFINE_DEVICE_TYPE(TMS2170, tms2170_cpu_device, "tms2170", "Texas Instruments TMS2170") // high voltage version, 1 R pin removed for Vpp
DEFINE_DEVICE_TYPE(TMS2300, tms2300_cpu_device, "tms2300", "Texas Instruments TMS2300") // 40-pin DIP, 15 R pins, J pins
DEFINE_DEVICE_TYPE(TMS2370, tms2370_cpu_device, "tms2370", "Texas Instruments TMS2370") // high voltage version, 1 R pin removed for Vpp


// device definitions
tms2100_cpu_device::tms2100_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	tms2100_cpu_device(mconfig, TMS2100, tag, owner, clock, 8 /* o pins */, 7 /* r pins */, 6 /* pc bits */, 8 /* byte width */, 3 /* x width */, 4 /* stack levels */, 11 /* rom width */, address_map_constructor(FUNC(tms2100_cpu_device::rom_11bit), this), 7 /* ram width */, address_map_constructor(FUNC(tms2100_cpu_device::ram_7bit), this))
{ }

tms2100_cpu_device::tms2100_cpu_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, u8 o_pins, u8 r_pins, u8 pc_bits, u8 byte_bits, u8 x_bits, u8 stack_levels, int rom_width, address_map_constructor rom_map, int ram_width, address_map_constructor ram_map) :
	tms1100_cpu_device(mconfig, type, tag, owner, clock, o_pins, r_pins, pc_bits, byte_bits, x_bits, stack_levels, rom_width, rom_map, ram_width, ram_map)
{ }

tms2170_cpu_device::tms2170_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	tms2100_cpu_device(mconfig, TMS2170, tag, owner, clock, 8, 6, 6, 8, 3, 4, 11, address_map_constructor(FUNC(tms2170_cpu_device::rom_11bit), this), 7, address_map_constructor(FUNC(tms2170_cpu_device::ram_7bit), this))
{ }

tms2300_cpu_device::tms2300_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	tms2300_cpu_device(mconfig, TMS2300, tag, owner, clock, 8, 15, 6, 8, 3, 4, 11, address_map_constructor(FUNC(tms2300_cpu_device::rom_11bit), this), 7, address_map_constructor(FUNC(tms2300_cpu_device::ram_7bit), this))
{ }

tms2300_cpu_device::tms2300_cpu_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, u8 o_pins, u8 r_pins, u8 pc_bits, u8 byte_bits, u8 x_bits, u8 stack_levels, int rom_width, address_map_constructor rom_map, int ram_width, address_map_constructor ram_map) :
	tms2100_cpu_device(mconfig, type, tag, owner, clock, o_pins, r_pins, pc_bits, byte_bits, x_bits, stack_levels, rom_width, rom_map, ram_width, ram_map)
{ }

tms2370_cpu_device::tms2370_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	tms2300_cpu_device(mconfig, TMS2370, tag, owner, clock, 8, 14, 6, 8, 3, 4, 11, address_map_constructor(FUNC(tms2370_cpu_device::rom_11bit), this), 7, address_map_constructor(FUNC(tms2370_cpu_device::ram_7bit), this))
{ }


// machine configs
void tms2100_cpu_device::device_add_mconfig(machine_config &config)
{
	// microinstructions PLA, output PLA
	PLA(config, "mpla", 8, 16, 26).set_format(pla_device::FMT::BERKELEY);
	PLA(config, "opla", 5, 8, 32).set_format(pla_device::FMT::BERKELEY);
}


// disasm
std::unique_ptr<util::disasm_interface> tms2100_cpu_device::create_disassembler()
{
	return std::make_unique<tms2100_disassembler>();
}


// device_reset
void tms2100_cpu_device::device_reset()
{
	tms1100_cpu_device::device_reset();

	// changed/added fixed instructions
}
