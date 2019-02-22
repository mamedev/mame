// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Coleco AdamLink 300 Baud Modem emulation

**********************************************************************/

#include "emu.h"
#include "adamlink.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(ADAMLINK, adamlink_device, "adamlink", "AdamLink modem")



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  adamlink_device - constructor
//-------------------------------------------------

adamlink_device::adamlink_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, ADAMLINK, tag, owner, clock)
	, device_adam_expansion_slot_card_interface(mconfig, *this)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void adamlink_device::device_start()
{
}


//-------------------------------------------------
//  adam_bd_r - buffered data read
//-------------------------------------------------

uint8_t adamlink_device::adam_bd_r(offs_t offset, uint8_t data, int bmreq, int biorq, int aux_rom_cs, int cas1, int cas2)
{
	if (!biorq)
	{
		switch (offset)
		{
		case 0x5e:
			break;

		case 0x5f:
			break;
		}
	}

	return data;
}


//-------------------------------------------------
//  adam_bd_w - buffered data write
//-------------------------------------------------

void adamlink_device::adam_bd_w(offs_t offset, uint8_t data, int bmreq, int biorq, int aux_rom_cs, int cas1, int cas2)
{
	if (!biorq)
	{
		switch (offset)
		{
		case 0x5e:
			break;

		case 0x5f:
			break;
		}
	}
}
