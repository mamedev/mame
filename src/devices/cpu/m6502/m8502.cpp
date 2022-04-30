// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/***************************************************************************

    m8502.c

    6510 derivative, capable of running at 2MHz.

***************************************************************************/

#include "emu.h"
#include "m8502.h"

DEFINE_DEVICE_TYPE(M8502, m8502_device, "m8502", "MOS Technology 8502")

m8502_device::m8502_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	m6510_device(mconfig, M8502, tag, owner, clock)
{
}
