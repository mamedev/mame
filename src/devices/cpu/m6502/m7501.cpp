// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/***************************************************************************

    m7501.cpp

    6510 derivative, essentially identical.  Also known as the 8501.

***************************************************************************/

#include "emu.h"
#include "m7501.h"

DEFINE_DEVICE_TYPE(M7501, m7501_device, "m7501", "MOS Technology 7501")

m7501_device::m7501_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	m6510_device(mconfig, M7501, tag, owner, clock)
{
}
