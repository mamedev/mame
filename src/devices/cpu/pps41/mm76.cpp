// license:BSD-3-Clause
// copyright-holders:hap
/*

  Rockwell MM76 MCU

*/

#include "emu.h"
#include "mm76.h"

#include "pps41d.h"


DEFINE_DEVICE_TYPE(MM76, mm76_device, "mm76", "Rockwell MM76")
DEFINE_DEVICE_TYPE(MM76L, mm76l_device, "mm76l", "Rockwell MM76L")
DEFINE_DEVICE_TYPE(MM76E, mm76e_device, "mm76e", "Rockwell MM76E")
DEFINE_DEVICE_TYPE(MM76EL, mm76el_device, "mm76el", "Rockwell MM76EL")


// constructor
mm76_device::mm76_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	mm76_device(mconfig, MM76, tag, owner, clock, 10, address_map_constructor(FUNC(mm76_device::program_0_6k), this), 6, address_map_constructor(FUNC(mm76_device::data_48x4), this))
{ }

mm76_device::mm76_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, int prgwidth, address_map_constructor program, int datawidth, address_map_constructor data) :
	pps41_base_device(mconfig, type, tag, owner, clock, prgwidth, program, datawidth, data)
{ }

mm76l_device::mm76l_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	mm76_device(mconfig, MM76L, tag, owner, clock, 10, address_map_constructor(FUNC(mm76l_device::program_0_6k), this), 6, address_map_constructor(FUNC(mm76l_device::data_48x4), this))
{ }

mm76e_device::mm76e_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	mm76e_device(mconfig, MM76E, tag, owner, clock, 10, address_map_constructor(FUNC(mm76e_device::program_1k), this), 6, address_map_constructor(FUNC(mm76e_device::data_48x4), this))
{ }

mm76e_device::mm76e_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, int prgwidth, address_map_constructor program, int datawidth, address_map_constructor data) :
	mm76_device(mconfig, type, tag, owner, clock, prgwidth, program, datawidth, data)
{ }

mm76el_device::mm76el_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	mm76e_device(mconfig, MM76EL, tag, owner, clock, 10, address_map_constructor(FUNC(mm76el_device::program_1k), this), 6, address_map_constructor(FUNC(mm76el_device::data_48x4), this))
{ }


// internal memory maps
void mm76_device::program_0_6k(address_map &map)
{
	map(0x000, 0x17f).mirror(0x200).rom();
	map(0x180, 0x1ff).rom();
	map(0x380, 0x3ff).rom();
}

void mm76e_device::program_1k(address_map &map)
{
	map(0x000, 0x3ff).rom();
}

void mm76_device::data_48x4(address_map &map)
{
	map(0x00, 0x2f).ram();
}


// machine config
void mm76_device::device_add_mconfig(machine_config &config)
{
	PLA(config, "opla", 5, 8, 17).set_format(pla_device::FMT::BERKELEY);
}


// disasm
std::unique_ptr<util::disasm_interface> mm76_device::create_disassembler()
{
	return std::make_unique<mm76_disassembler>();
}


// initialize
void mm76_device::device_start()
{
	pps41_base_device::device_start();
	m_stack_levels = 1;
	m_d_pins = 10;
}

void mm76_device::device_reset()
{
	pps41_base_device::device_reset();
}


//-------------------------------------------------
//  execute
//-------------------------------------------------

void mm76_device::execute_one()
{
	op_nop();
}
