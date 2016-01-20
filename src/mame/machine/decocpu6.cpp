// license:BSD-3-Clause
// copyright-holders:David Haywood
/* apparently Deco CPU-6 used by ProGolf
 just seems to be a bitswap on the opcodes like 222, but not the same one
 not a complex scheme like CPU-7?
*/


#include "decocpu6.h"

deco_cpu6_device::deco_cpu6_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock) :
	m6502_device(mconfig, DECO_CPU6, "DECO CPU-6", tag, owner, clock, "decocpu6", __FILE__)
{
}

void deco_cpu6_device::device_start()
{
	mintf = new mi_decrypt;
	init();
}

void deco_cpu6_device::device_reset()
{
	m6502_device::device_reset();
}

UINT8 deco_cpu6_device::mi_decrypt::read_sync(UINT16 adr)
{
	if (adr&1)
		return BITSWAP8(direct->read_byte(adr),6,4,7,5,3,2,1,0);
	else
		return direct->read_byte(adr);
}
