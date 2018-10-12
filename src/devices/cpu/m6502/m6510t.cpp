// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/***************************************************************************

    m6510t.c

    6510 with the full 8 i/o pins at the expense of the NMI and RDY lines.

***************************************************************************/

#include "emu.h"
#include "m6510t.h"

DEFINE_DEVICE_TYPE(M6510T, m6510t_device, "m6510t", "MOS Technology M6510T")

m6510t_device::m6510t_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	m6510_device(mconfig, M6510T, tag, owner, clock)
{
}
