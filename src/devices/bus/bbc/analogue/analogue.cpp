// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    BBC analogue port emulation

**********************************************************************/

#include "emu.h"
#include "analogue.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(BBC_ANALOGUE_SLOT, bbc_analogue_slot_device, "bbc_analogue_slot", "BBC Micro Analogue port")



//**************************************************************************
//  DEVICE BBC_ANALOGUE PORT INTERFACE
//**************************************************************************

//-------------------------------------------------
//  device_bbc_analogue_interface - constructor
//-------------------------------------------------

device_bbc_analogue_interface::device_bbc_analogue_interface(const machine_config &mconfig, device_t &device)
	: device_slot_card_interface(mconfig, device)
{
	m_slot = dynamic_cast<bbc_analogue_slot_device *>(device.owner());
}


//-------------------------------------------------
//  ~device_bbc_analogue_interface - destructor
//-------------------------------------------------

device_bbc_analogue_interface::~device_bbc_analogue_interface()
{
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  bbc_analogue_slot_device - constructor
//-------------------------------------------------

bbc_analogue_slot_device::bbc_analogue_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, BBC_ANALOGUE_SLOT, tag, owner, clock),
	device_slot_interface(mconfig, *this)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void bbc_analogue_slot_device::device_start()
{
	m_card = dynamic_cast<device_bbc_analogue_interface *>(get_card_device());
}

uint8_t bbc_analogue_slot_device::ch_r(int channel)
{
	if (m_card)
		return m_card->ch_r(channel);
	else
		return 0x00;
}

uint8_t bbc_analogue_slot_device::pb_r()
{
	if (m_card)
		return m_card->pb_r();
	else
		return 0x30;
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void bbc_analogue_slot_device::device_reset()
{
	if (get_card_device())
	{
		get_card_device()->reset();
	}
}


//-------------------------------------------------
//  SLOT_INTERFACE( bbc_analogue_devices )
//-------------------------------------------------


// slot devices
#include "joystick.h"
//#include "quinkey.h"


SLOT_INTERFACE_START( bbc_analogue_devices )
	SLOT_INTERFACE("acornjoy",    BBC_ACORNJOY)         /* Acorn ANH01 Joysticks */
	SLOT_INTERFACE("voltmace3b",  BBC_VOLTMACE3B)       /* Voltmace Delta 3b "Twin" Joysticks */
//  SLOT_INTERFACE("quinkey",   BBC_QUINKEY)          /* Microwriter Quinkey */
SLOT_INTERFACE_END
