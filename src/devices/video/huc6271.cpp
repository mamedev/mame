// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/***************************************************************************

    Hudson/NEC HuC6271 "Rainbow" device

***************************************************************************/

#include "emu.h"
#include "huc6271.h"



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// device type definition
const device_type huc6271 = &device_creator<huc6271_device>;


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  huc6271_device - constructor
//-------------------------------------------------

huc6271_device::huc6271_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, huc6271, "HuC6271 \"Rainbow\"", tag, owner, clock, "huc6271", __FILE__)
{
}

DEVICE_ADDRESS_MAP_START( regs, 16, huc6271_device )
	AM_RANGE(0x00, 0x01) AM_WRITENOP // hscroll
	AM_RANGE(0x02, 0x03) AM_WRITENOP // control
	AM_RANGE(0x04, 0x05) AM_WRITENOP // hsync
	AM_RANGE(0x06, 0x07) AM_WRITENOP // base Y
	AM_RANGE(0x08, 0x09) AM_WRITENOP // base U
	AM_RANGE(0x0a, 0x0b) AM_WRITENOP // base V
ADDRESS_MAP_END

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void huc6271_device::device_start()
{
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void huc6271_device::device_reset()
{
}


//**************************************************************************
//  READ/WRITE HANDLERS
//**************************************************************************

