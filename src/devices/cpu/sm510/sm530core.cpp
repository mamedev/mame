// license:BSD-3-Clause
// copyright-holders:hap
/*

  Sharp SM530 MCU core implementation

  TODO:
  - everything

*/

#include "emu.h"
#include "sm530.h"
#include "sm510d.h"
#include "debugger.h"


// MCU types
DEFINE_DEVICE_TYPE(SM530, sm530_device, "sm530", "Sharp SM530") // x
DEFINE_DEVICE_TYPE(SM531, sm531_device, "sm531", "Sharp SM531") // x


// internal memory maps
void sm530_device::program_2k(address_map &map)
{
	map(0x000, 0x7ff).rom();
}

void sm530_device::data_64_24x4(address_map &map)
{
	map(0x00, 0x3f).ram();
	map(0x40, 0x4b).ram();
	map(0x50, 0x5b).ram();
}


// disasm
std::unique_ptr<util::disasm_interface> sm530_device::create_disassembler()
{
	return std::make_unique<sm530_disassembler>();
}


// device definitions
sm530_device::sm530_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: sm530_device(mconfig, SM530, tag, owner, clock, 1 /* stack levels */, 11 /* prg width */, address_map_constructor(FUNC(sm530_device::program_2k), this), 7 /* data width */, address_map_constructor(FUNC(sm530_device::data_64_24x4), this))
{
}

sm530_device::sm530_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, int stack_levels, int prgwidth, address_map_constructor program, int datawidth, address_map_constructor data)
	: sm511_device(mconfig, type, tag, owner, clock, stack_levels, prgwidth, program, datawidth, data)
{
}

sm531_device::sm531_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: sm530_device(mconfig, SM531, tag, owner, clock, 1, 11, address_map_constructor(FUNC(sm531_device::program_2k), this), 7, address_map_constructor(FUNC(sm531_device::data_64_24x4), this))
{
}



//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void sm530_device::device_reset()
{
	sm510_base_device::device_reset();
}



//-------------------------------------------------
//  execute
//-------------------------------------------------

void sm530_device::get_opcode_param()
{
}

void sm530_device::execute_one()
{
}
