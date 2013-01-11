/**********************************************************************

    Commodore 64 User Port emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************/

#include "machine/c64user.h"



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

const device_type C64_USER_PORT = &device_creator<c64_user_port_device>;



//**************************************************************************
//  CARD INTERFACE
//**************************************************************************

//-------------------------------------------------
//  device_c64_user_port_interface - constructor
//-------------------------------------------------

device_c64_user_port_interface::device_c64_user_port_interface(const machine_config &mconfig, device_t &device)
	: device_slot_card_interface(mconfig,device)
{
	m_slot = dynamic_cast<c64_user_port_device *>(device.owner());
}


//-------------------------------------------------
//  ~device_c64_user_port_interface - destructor
//-------------------------------------------------

device_c64_user_port_interface::~device_c64_user_port_interface()
{
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  c64_user_port_device - constructor
//-------------------------------------------------

c64_user_port_device::c64_user_port_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
		device_t(mconfig, C64_USER_PORT, "C64 user port", tag, owner, clock),
		device_slot_interface(mconfig, *this)
{
}


//-------------------------------------------------
//  c64_user_port_device - destructor
//-------------------------------------------------

c64_user_port_device::~c64_user_port_device()
{
}


//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void c64_user_port_device::device_config_complete()
{
	// inherit a copy of the static data
	const c64_user_port_interface *intf = reinterpret_cast<const c64_user_port_interface *>(static_config());
	if (intf != NULL)
	{
		*static_cast<c64_user_port_interface *>(this) = *intf;
	}

	// or initialize to defaults if none provided
	else
	{
		memset(&m_out_cnt1_cb, 0, sizeof(m_out_cnt1_cb));
		memset(&m_out_sp1_cb, 0, sizeof(m_out_sp1_cb));
		memset(&m_out_cnt2_cb, 0, sizeof(m_out_cnt2_cb));
		memset(&m_out_sp2_cb, 0, sizeof(m_out_sp2_cb));
		memset(&m_out_flag2_cb, 0, sizeof(m_out_flag2_cb));
		memset(&m_out_reset_cb, 0, sizeof(m_out_reset_cb));
	}
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void c64_user_port_device::device_start()
{
	m_cart = dynamic_cast<device_c64_user_port_interface *>(get_card_device());

	// resolve callbacks
	m_out_cnt1_func.resolve(m_out_cnt1_cb, *this);
	m_out_sp1_func.resolve(m_out_sp1_cb, *this);
	m_out_cnt2_func.resolve(m_out_cnt2_cb, *this);
	m_out_sp2_func.resolve(m_out_sp2_cb, *this);
	m_out_flag2_func.resolve(m_out_flag2_cb, *this);
	m_out_reset_func.resolve(m_out_reset_cb, *this);
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void c64_user_port_device::device_reset()
{
	port_reset_w(ASSERT_LINE);
	port_reset_w(CLEAR_LINE);
}


READ8_MEMBER( c64_user_port_device::pb_r ) { UINT8 data = 0xff; if (m_cart != NULL) data = m_cart->c64_pb_r(space, offset); return data; }
WRITE8_MEMBER( c64_user_port_device::pb_w ) { if (m_cart != NULL) m_cart->c64_pb_w(space, offset, data); }
READ_LINE_MEMBER( c64_user_port_device::pa2_r ) { int state = 1; if (m_cart != NULL) state = m_cart->c64_pa2_r(); return state; }
WRITE_LINE_MEMBER( c64_user_port_device::pa2_w ) { if (m_cart != NULL) m_cart->c64_pa2_w(state); }
WRITE_LINE_MEMBER( c64_user_port_device::pc2_w ) { if (m_cart != NULL) m_cart->c64_pc2_w(state); }
WRITE_LINE_MEMBER( c64_user_port_device::atn_w ) { if (m_cart != NULL) m_cart->c64_atn_w(state); }
WRITE_LINE_MEMBER( c64_user_port_device::port_reset_w ) { if (m_cart != NULL) m_cart->c64_reset_w(state); }

WRITE_LINE_MEMBER( c64_user_port_device::cnt1_w ) { m_out_cnt1_func(state); }
WRITE_LINE_MEMBER( c64_user_port_device::sp1_w ) { m_out_sp1_func(state); }
WRITE_LINE_MEMBER( c64_user_port_device::cnt2_w ) { m_out_cnt2_func(state); }
WRITE_LINE_MEMBER( c64_user_port_device::sp2_w ) { m_out_sp2_func(state); }
WRITE_LINE_MEMBER( c64_user_port_device::flag2_w ) { m_out_flag2_func(state); }
WRITE_LINE_MEMBER( c64_user_port_device::reset_w ) { m_out_reset_func(state); }
