/**********************************************************************

    Commodore Plus/4 User Port emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************/

#include "machine/plus4user.h"



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

const device_type PLUS4_USER_PORT = &device_creator<plus4_user_port_device>;



//**************************************************************************
//  CARD INTERFACE
//**************************************************************************

//-------------------------------------------------
//  device_plus4_user_port_interface - constructor
//-------------------------------------------------

device_plus4_user_port_interface::device_plus4_user_port_interface(const machine_config &mconfig, device_t &device)
	: device_slot_card_interface(mconfig,device)
{
	m_slot = dynamic_cast<plus4_user_port_device *>(device.owner());
}


//-------------------------------------------------
//  ~device_plus4_user_port_interface - destructor
//-------------------------------------------------

device_plus4_user_port_interface::~device_plus4_user_port_interface()
{
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  plus4_user_port_device - constructor
//-------------------------------------------------

plus4_user_port_device::plus4_user_port_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
		device_t(mconfig, PLUS4_USER_PORT, "User Port", tag, owner, clock),
		device_slot_interface(mconfig, *this)
{
}


//-------------------------------------------------
//  plus4_user_port_device - destructor
//-------------------------------------------------

plus4_user_port_device::~plus4_user_port_device()
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void plus4_user_port_device::device_start()
{
	m_cart = dynamic_cast<device_plus4_user_port_interface *>(get_card_device());
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void plus4_user_port_device::device_reset()
{
	breset_w(ASSERT_LINE);
	breset_w(CLEAR_LINE);
}


READ8_MEMBER( plus4_user_port_device::p_r ) { UINT8 data = 0xff; if (m_cart != NULL) data = m_cart->plus4_p_r(); return data; }
WRITE8_MEMBER( plus4_user_port_device::p_w ) { if (m_cart != NULL) m_cart->plus4_p_w(data); }

READ_LINE_MEMBER( plus4_user_port_device::rxd_r ) { int state = 1; if (m_cart != NULL) state = m_cart->plus4_rxd_r(); return state; }
READ_LINE_MEMBER( plus4_user_port_device::dcd_r ) { int state = 1; if (m_cart != NULL) state = m_cart->plus4_dcd_r(); return state; }
READ_LINE_MEMBER( plus4_user_port_device::dsr_r ) { int state = 1; if (m_cart != NULL) state = m_cart->plus4_dsr_r(); return state; }

WRITE_LINE_MEMBER( plus4_user_port_device::txd_w ) { if (m_cart != NULL) m_cart->plus4_txd_w(state); }
WRITE_LINE_MEMBER( plus4_user_port_device::dtr_w ) { if (m_cart != NULL) m_cart->plus4_dtr_w(state); }
WRITE_LINE_MEMBER( plus4_user_port_device::rts_w ) { if (m_cart != NULL) m_cart->plus4_rts_w(state); }
WRITE_LINE_MEMBER( plus4_user_port_device::rxc_w ) { if (m_cart != NULL) m_cart->plus4_rxc_w(state); }
WRITE_LINE_MEMBER( plus4_user_port_device::atn_w ) { if (m_cart != NULL) m_cart->plus4_atn_w(state); }
WRITE_LINE_MEMBER( plus4_user_port_device::breset_w ) { if (m_cart != NULL) m_cart->plus4_breset_w(state); }
