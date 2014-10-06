// license:BSD-3-Clause
// copyright-holders:hap
/**********************************************************************

    NEC uPD71054 programmable timer/counter

    TODO:
    - x

**********************************************************************/

#include "machine/upd71054.h"


const device_type UPD71054 = &device_creator<upd71054_device>;

//-------------------------------------------------
//  upd71054_device - constructor
//-------------------------------------------------

upd71054_device::upd71054_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, UPD71054, "NEC uPD71054", tag, owner, clock, "upd71054", __FILE__)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void upd71054_device::device_start()
{
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void upd71054_device::device_reset()
{
}
