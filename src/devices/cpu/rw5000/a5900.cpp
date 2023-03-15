// license:BSD-3-Clause
// copyright-holders:hap
/*

  Rockwell A5900 MCU

TODO:
- what happens when 0 or >1 keys are held down on READ?

*/

#include "emu.h"
#include "a5900.h"


DEFINE_DEVICE_TYPE(A5900, a5900_cpu_device, "a5900", "Rockwell A5900")


// constructor
a5900_cpu_device::a5900_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	a5000_cpu_device(mconfig, A5900, tag, owner, clock, 9, address_map_constructor(FUNC(a5900_cpu_device::program_512x8), this), 6, address_map_constructor(FUNC(a5900_cpu_device::data_45x4), this))
{ }


// internal memory maps
void a5900_cpu_device::program_512x8(address_map &map)
{
	map(0x000, 0x1ff).rom();
}


//-------------------------------------------------
//  changed opcodes (no need for separate file)
//-------------------------------------------------

void a5900_cpu_device::op_read()
{
	// READ: add _KB (prioritized) to A, skip next on no overflow
	m_a += ~((count_leading_zeros_32(m_read_kb() & 0xf) - 28) & 3) & 0xf;
	m_skip = !BIT(m_a, 4);
	m_a &= 0xf;
}
