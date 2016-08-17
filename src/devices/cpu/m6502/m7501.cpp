// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/***************************************************************************

    m7501.c

    6510 derivative, essentially identical.  Also known as the 8501.

***************************************************************************/

#include "emu.h"
#include "m7501.h"

const device_type M7501 = &device_creator<m7501_device>;

m7501_device::m7501_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	m6510_device(mconfig, M7501, "M7501", tag, owner, clock, "m7501", __FILE__)
{
}
