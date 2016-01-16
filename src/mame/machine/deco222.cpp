// license:BSD-3-Clause
// copyright-holders:David Haywood

/* Data East encrypted CPU 222, aka C10707?
 also sometimes implemented as basic logic outside the CPU on early revs and bootlegs */


#include "deco222.h"


deco_222_device::deco_222_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock) :
	m6502_device(mconfig, DECO_222, "DECO 222", tag, owner, clock, "deco222", __FILE__)
{
}

void deco_222_device::device_start()
{
	mintf = new mi_decrypt;
	init();
}

void deco_222_device::device_reset()
{
	m6502_device::device_reset();
	static_cast<mi_decrypt *>(mintf)->had_written = false;
}

UINT8 deco_222_device::mi_decrypt::read_sync(UINT16 adr)
{
	return BITSWAP8(direct->read_byte(adr) ,7,5,6,4,3,2,1,0);
}



deco_c10707_device::deco_c10707_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock) :
	m6502_device(mconfig, DECO_C10707, "DECO C10707", tag, owner, clock, "decoc10707", __FILE__)
{
}

void deco_c10707_device::device_start()
{
	mintf = new mi_decrypt;
	init();
}

void deco_c10707_device::device_reset()
{
	m6502_device::device_reset();
	static_cast<mi_decrypt *>(mintf)->had_written = false;
}

UINT8 deco_c10707_device::mi_decrypt::read_sync(UINT16 adr)
{
	return BITSWAP8(direct->read_byte(adr) ,7,5,6,4,3,2,1,0);
}
