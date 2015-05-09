// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/***************************************************************************

    m65sc02.c

    Rockwell-class 65c02 with internal static registers, making clock stoppable?

***************************************************************************/

#include "emu.h"
#include "m65sc02.h"

const device_type M65SC02 = &device_creator<m65sc02_device>;

m65sc02_device::m65sc02_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	r65c02_device(mconfig, M65SC02, "M65SC02", tag, owner, clock, "m65sc02", __FILE__)
{
}
