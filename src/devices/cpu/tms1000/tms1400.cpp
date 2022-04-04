// license:BSD-3-Clause
// copyright-holders:hap
/*

  TMS1000 family - TMS1400, TMS1470, TMS1600, TMS1670

  TODO:
  - emulate TMS1600 L-pins

*/

#include "emu.h"
#include "tms1400.h"

// TMS1400 follows the TMS1100, it doubles the ROM size again (4 chapters of 16 pages), and adds a 3-level callstack
// - rotate the view and mirror the OR-mask to get the proper layout of the mpla, the default is identical to tms1100
// - the opla size is increased from 20 to 32 terms
DEFINE_DEVICE_TYPE(TMS1400, tms1400_cpu_device, "tms1400", "Texas Instruments TMS1400") // 28-pin DIP, 11 R pins (TMS1400CR is same, but with TMS1100 pinout)
DEFINE_DEVICE_TYPE(TMS1470, tms1470_cpu_device, "tms1470", "Texas Instruments TMS1470") // high voltage version, 1 R pin removed for Vdd

// TMS1600 adds more I/O to the TMS1400, input pins are doubled with added L1,2,4,8
// - rotate the view and mirror the OR-mask to get the proper layout of the mpla, the default is identical to tms1100
// - the opla size is increased from 20 to 32 terms
DEFINE_DEVICE_TYPE(TMS1600, tms1600_cpu_device, "tms1600", "Texas Instruments TMS1600") // 40-pin DIP, 16 R pins
DEFINE_DEVICE_TYPE(TMS1670, tms1670_cpu_device, "tms1670", "Texas Instruments TMS1670") // high voltage version


// internal memory maps
void tms1400_cpu_device::program_12bit_8(address_map &map)
{
	map(0x000, 0xfff).rom();
}


// device definitions
tms1400_cpu_device::tms1400_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: tms1400_cpu_device(mconfig, TMS1400, tag, owner, clock, 8 /* o pins */, 11 /* r pins */, 6 /* pc bits */, 8 /* byte width */, 3 /* x width */, 12 /* prg width */, address_map_constructor(FUNC(tms1400_cpu_device::program_12bit_8), this), 7 /* data width */, address_map_constructor(FUNC(tms1400_cpu_device::data_128x4), this))
{
}

tms1400_cpu_device::tms1400_cpu_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, u8 o_pins, u8 r_pins, u8 pc_bits, u8 byte_bits, u8 x_bits, int prgwidth, address_map_constructor program, int datawidth, address_map_constructor data)
	: tms1100_cpu_device(mconfig, type, tag, owner, clock, o_pins, r_pins, pc_bits, byte_bits, x_bits, prgwidth, program, datawidth, data)
{
}

tms1470_cpu_device::tms1470_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: tms1400_cpu_device(mconfig, TMS1470, tag, owner, clock, 8, 10, 6, 8, 3, 12, address_map_constructor(FUNC(tms1470_cpu_device::program_12bit_8), this), 7, address_map_constructor(FUNC(tms1470_cpu_device::data_128x4), this))
{
}


tms1600_cpu_device::tms1600_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: tms1600_cpu_device(mconfig, TMS1600, tag, owner, clock, 8, 16, 6, 8, 3, 12, address_map_constructor(FUNC(tms1600_cpu_device::program_12bit_8), this), 7, address_map_constructor(FUNC(tms1600_cpu_device::data_128x4), this))
{
}

tms1600_cpu_device::tms1600_cpu_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, u8 o_pins, u8 r_pins, u8 pc_bits, u8 byte_bits, u8 x_bits, int prgwidth, address_map_constructor program, int datawidth, address_map_constructor data)
	: tms1400_cpu_device(mconfig, type, tag, owner, clock, o_pins, r_pins, pc_bits, byte_bits, x_bits, prgwidth, program, datawidth, data)
{
}

tms1670_cpu_device::tms1670_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: tms1600_cpu_device(mconfig, TMS1670, tag, owner, clock, 8, 16, 6, 8, 3, 12, address_map_constructor(FUNC(tms1670_cpu_device::program_12bit_8), this), 7, address_map_constructor(FUNC(tms1670_cpu_device::data_128x4), this))
{
}


// machine configs
void tms1400_cpu_device::device_add_mconfig(machine_config &config)
{
	// microinstructions PLA, output PLA
	PLA(config, "mpla", 8, 16, 30).set_format(pla_device::FMT::BERKELEY);
	PLA(config, "opla", 5, 8, 32).set_format(pla_device::FMT::BERKELEY);
}


// device_reset
void tms1400_cpu_device::device_reset()
{
	tms1100_cpu_device::device_reset();

	// small differences in 00-3f area
	m_fixed_decode[0x0b] = F_TPC;
}
