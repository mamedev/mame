// license:BSD-3-Clause
// copyright-holders:hap
/*

  Rockwell MM76 MCU

*/

#include "emu.h"
#include "mm76.h"

#include "pps41d.h"


DEFINE_DEVICE_TYPE(MM76, mm76_device, "mm76", "Rockwell MM76")


// internal memory maps
void mm76_device::program_map(address_map &map)
{
	map(0x0000, 0x01ff).rom();
	map(0x0380, 0x03ff).rom();
}

void mm76_device::data_map(address_map &map)
{
	map(0x00, 0x02f).ram();
}


// device definitions
mm76_device::mm76_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	pps41_base_device(mconfig, MM76, tag, owner, clock, 10, address_map_constructor(FUNC(mm76_device::program_map), this), 6, address_map_constructor(FUNC(mm76_device::data_map), this))
{ }


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
