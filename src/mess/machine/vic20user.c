/**********************************************************************

    Commodore VIC-20 User Port emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************/

#include "machine/vic20user.h"



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

const device_type VIC20_USER_PORT = &device_creator<vic20_user_port_device>;



//**************************************************************************
//  CARD INTERFACE
//**************************************************************************

//-------------------------------------------------
//  device_vic20_user_port_interface - constructor
//-------------------------------------------------

device_vic20_user_port_interface::device_vic20_user_port_interface(const machine_config &mconfig, device_t &device)
	: device_slot_card_interface(mconfig,device)
{
	m_slot = dynamic_cast<vic20_user_port_device *>(device.owner());
}


//-------------------------------------------------
//  ~device_vic20_user_port_interface - destructor
//-------------------------------------------------

device_vic20_user_port_interface::~device_vic20_user_port_interface()
{
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  vic20_user_port_device - constructor
//-------------------------------------------------

vic20_user_port_device::vic20_user_port_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
        device_t(mconfig, VIC20_USER_PORT, "VIC-20 user port", tag, owner, clock),
		device_slot_interface(mconfig, *this)
{
}


//-------------------------------------------------
//  vic20_user_port_device - destructor
//-------------------------------------------------

vic20_user_port_device::~vic20_user_port_device()
{
}


//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void vic20_user_port_device::device_config_complete()
{
	// inherit a copy of the static data
	const vic20_user_port_interface *intf = reinterpret_cast<const vic20_user_port_interface *>(static_config());
	if (intf != NULL)
	{
		*static_cast<vic20_user_port_interface *>(this) = *intf;
	}

	// or initialize to defaults if none provided
	else
	{
    	memset(&m_out_cb1_cb, 0, sizeof(m_out_cb1_cb));
    	memset(&m_out_cb2_cb, 0, sizeof(m_out_cb2_cb));
    	memset(&m_out_reset_cb, 0, sizeof(m_out_reset_cb));
	}
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void vic20_user_port_device::device_start()
{
	m_cart = dynamic_cast<device_vic20_user_port_interface *>(get_card_device());

	// resolve callbacks
	m_out_cb1_func.resolve(m_out_cb1_cb, *this);
	m_out_cb2_func.resolve(m_out_cb2_cb, *this);
	m_out_reset_func.resolve(m_out_reset_cb, *this);
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void vic20_user_port_device::device_reset()
{
	port_reset_w(ASSERT_LINE);
	port_reset_w(CLEAR_LINE);
}


READ8_MEMBER( vic20_user_port_device::pb_r ) { UINT8 data = 0xff; if (m_cart != NULL) data = m_cart->vic20_pb_r(space, offset); return data; }
WRITE8_MEMBER( vic20_user_port_device::pb_w ) { if (m_cart != NULL) m_cart->vic20_pb_w(space, offset, data); }
READ_LINE_MEMBER( vic20_user_port_device::joy0_r ) { int state = 1; if (m_cart != NULL) state = m_cart->vic20_joy0_r(); return state; }
READ_LINE_MEMBER( vic20_user_port_device::joy1_r ) { int state = 1; if (m_cart != NULL) state = m_cart->vic20_joy1_r(); return state; }
READ_LINE_MEMBER( vic20_user_port_device::joy2_r ) { int state = 1; if (m_cart != NULL) state = m_cart->vic20_joy2_r(); return state; }
READ_LINE_MEMBER( vic20_user_port_device::light_pen_r ) { int state = 1; if (m_cart != NULL) state = m_cart->vic20_light_pen_r(); return state; }
READ_LINE_MEMBER( vic20_user_port_device::cassette_switch_r ) { int state = 1; if (m_cart != NULL) state = m_cart->vic20_cassette_switch_r(); return state; }
WRITE_LINE_MEMBER( vic20_user_port_device::cb1_w ) { if (m_cart != NULL) m_cart->vic20_cb1_w(state); }
WRITE_LINE_MEMBER( vic20_user_port_device::cb2_w ) { if (m_cart != NULL) m_cart->vic20_cb2_w(state); }
WRITE_LINE_MEMBER( vic20_user_port_device::atn_w ) { if (m_cart != NULL) m_cart->vic20_atn_w(state); }
WRITE_LINE_MEMBER( vic20_user_port_device::port_reset_w ) { if (m_cart != NULL) m_cart->vic20_reset_w(state); }

WRITE_LINE_MEMBER( vic20_user_port_device::via_cb1_w ) { m_out_cb1_func(state); }
WRITE_LINE_MEMBER( vic20_user_port_device::via_cb2_w ) { m_out_cb2_func(state); }
WRITE_LINE_MEMBER( vic20_user_port_device::reset_w ) { m_out_reset_func(state); }
