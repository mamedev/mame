// license:GPL-2.0+
// copyright-holders:Dirk Best
/**********************************************************************

    EPSON SIO port emulation

**********************************************************************/

#include "epson_sio.h"

// supported devices
#include "pf10.h"
#include "tf20.h"


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

const device_type EPSON_SIO = &device_creator<epson_sio_device>;


//**************************************************************************
//  CARD INTERFACE
//**************************************************************************

//-------------------------------------------------
//  device_epson_sio_interface - constructor
//-------------------------------------------------

device_epson_sio_interface::device_epson_sio_interface(const machine_config &mconfig, device_t &device) :
	device_slot_card_interface(mconfig, device)
{
	m_slot = dynamic_cast<epson_sio_device *>(device.owner());
}


//-------------------------------------------------
//  ~device_epson_sio_interface - destructor
//-------------------------------------------------

device_epson_sio_interface::~device_epson_sio_interface()
{
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  epson_sio_device - constructor
//-------------------------------------------------

epson_sio_device::epson_sio_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
		device_t(mconfig, EPSON_SIO, "EPSON SIO port", tag, owner, clock, "epson_sio", __FILE__),
		device_slot_interface(mconfig, *this),
		m_write_rx(*this),
		m_write_pin(*this)
{
}


//-------------------------------------------------
//  epson_sio_device - destructor
//-------------------------------------------------

epson_sio_device::~epson_sio_device()
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void epson_sio_device::device_start()
{
	m_cart = dynamic_cast<device_epson_sio_interface *>(get_card_device());

	m_write_rx.resolve_safe();
	m_write_pin.resolve_safe();
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void epson_sio_device::device_reset()
{
}


WRITE_LINE_MEMBER( epson_sio_device::tx_w )
{
	if (m_cart != NULL)
		m_cart->tx_w(state);
}

WRITE_LINE_MEMBER( epson_sio_device::pout_w )
{
	if (m_cart != NULL)
		m_cart->pout_w(state);
}


//**************************************************************************
//  SLOT INTERFACE
//**************************************************************************

SLOT_INTERFACE_START( epson_sio_devices )
	SLOT_INTERFACE("pf10", EPSON_PF10)
	SLOT_INTERFACE("tf20", EPSON_TF20)
SLOT_INTERFACE_END
