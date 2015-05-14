// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/***************************************************************************

    m6510t.c

    6510 with the full 8 i/o pins at the expense of the NMI and RDY lines.

***************************************************************************/

#include "emu.h"
#include "m6510t.h"

const device_type M6510T = &device_creator<m6510t_device>;

m6510t_device::m6510t_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	m6510_device(mconfig, M6510T, "M6510T", tag, owner, clock, "m6510t", __FILE__)
{
}
