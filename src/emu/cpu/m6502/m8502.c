// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/***************************************************************************

    m8502.c

    6510 derivative, capable of running at 2MHz.

***************************************************************************/

#include "emu.h"
#include "m8502.h"

const device_type M8502 = &device_creator<m8502_device>;

m8502_device::m8502_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	m6510_device(mconfig, M8502, "M8502", tag, owner, clock, "m8502", __FILE__)
{
}
