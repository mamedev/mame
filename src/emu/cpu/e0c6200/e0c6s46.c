// license:BSD-3-Clause
// copyright-holders:hap
/*

  Seiko Epson E0C6S46 MCU

*/

#include "e0c6s46.h"


const device_type E0C6S46 = &device_creator<e0c6s46_device>;


// internal memory maps
static ADDRESS_MAP_START(e0c6s46_program, AS_PROGRAM, 16, e0c6s46_device)
	AM_RANGE(0x0000, 0x17ff) AM_ROM
ADDRESS_MAP_END


static ADDRESS_MAP_START(e0c6s46_data, AS_DATA, 8, e0c6s46_device)
	AM_RANGE(0x0000, 0x027f) AM_RAM
ADDRESS_MAP_END


// device definitions
e0c6s46_device::e0c6s46_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: e0c6200_cpu_device(mconfig, E0C6S46, "E0C6S46", tag, owner, clock, 13, ADDRESS_MAP_NAME(e0c6s46_program), 12, ADDRESS_MAP_NAME(e0c6s46_data), "e0c6s46", __FILE__)
{ }




//-------------------------------------------------
//  execute
//-------------------------------------------------

void e0c6s46_device::execute_one()
{
	// E0C6S46 has no support for SLP opcode
	if (m_op == 0xff9)
		return;
	
	e0c6200_cpu_device::execute_one();
}
