// license:BSD-3-Clause
// copyright-holders:hap
/*

  Rockwell MM77/MM78 MCU

*/

#include "emu.h"
#include "mm78.h"

#include "pps41d.h"


DEFINE_DEVICE_TYPE(MM78, mm78_device, "mm78", "Rockwell MM78") // 2KB bytes ROM, 128 nibbles RAM
DEFINE_DEVICE_TYPE(MM78L, mm78l_device, "mm78l", "Rockwell MM78L") // low-power
DEFINE_DEVICE_TYPE(MM77, mm77_device, "mm77", "Rockwell MM77") // 1.3KB ROM, 96 nibbles RAM
DEFINE_DEVICE_TYPE(MM77L, mm77l_device, "mm77l", "Rockwell MM77L") // 1.5KB ROM, low-power


// constructor
mm78_device::mm78_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	mm78_device(mconfig, MM78, tag, owner, clock, 11, address_map_constructor(FUNC(mm78_device::program_2k), this), 7, address_map_constructor(FUNC(mm78_device::data_128x4), this))
{ }

mm78_device::mm78_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, int prgwidth, address_map_constructor program, int datawidth, address_map_constructor data) :
	mm76_device(mconfig, type, tag, owner, clock, prgwidth, program, datawidth, data)
{ }

mm78l_device::mm78l_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	mm78_device(mconfig, MM78L, tag, owner, clock, 11, address_map_constructor(FUNC(mm78l_device::program_2k), this), 7, address_map_constructor(FUNC(mm78l_device::data_128x4), this))
{ }

mm77_device::mm77_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	mm77_device(mconfig, MM77, tag, owner, clock, 11, address_map_constructor(FUNC(mm77_device::program_1_3k), this), 7, address_map_constructor(FUNC(mm77_device::data_96x4), this))
{ }

mm77_device::mm77_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, int prgwidth, address_map_constructor program, int datawidth, address_map_constructor data) :
	mm78_device(mconfig, type, tag, owner, clock, prgwidth, program, datawidth, data)
{ }

mm77l_device::mm77l_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	mm77_device(mconfig, MM77L, tag, owner, clock, 11, address_map_constructor(FUNC(mm77l_device::program_1_5k), this), 7, address_map_constructor(FUNC(mm77l_device::data_96x4), this))
{ }


// internal memory maps
void mm78_device::program_2k(address_map &map)
{
	map(0x000, 0x7ff).rom();
}

void mm77_device::program_1_3k(address_map &map)
{
	map(0x040, 0x1ff).rom();
	map(0x240, 0x3ff).rom();
	map(0x640, 0x7ff).rom();
}

void mm77l_device::program_1_5k(address_map &map)
{
	map(0x000, 0x3ff).rom();
	map(0x600, 0x7ff).rom();
}

void mm78_device::data_128x4(address_map &map)
{
	map(0x00, 0x7f).ram();
}

void mm77_device::data_96x4(address_map &map)
{
	map(0x00, 0x3f).ram();
	map(0x40, 0x47).mirror(0x18).ram(); // not to 0x50
	map(0x50, 0x57).ram();
	map(0x60, 0x67).mirror(0x18).ram(); // not to 0x70
	map(0x70, 0x77).ram();
}


// disasm
std::unique_ptr<util::disasm_interface> mm78_device::create_disassembler()
{
	return std::make_unique<mm78_disassembler>();
}


// initialize
void mm78_device::device_start()
{
	mm76_device::device_start();
	m_stack_levels = 2;

	state_add(++m_state_count, "X", m_x).formatstr("%01X");
}

void mm78_device::device_reset()
{
	mm76_device::device_reset();
}


//-------------------------------------------------
//  execute
//-------------------------------------------------

void mm78_device::execute_one()
{
}
