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
		device_t(mconfig, C64_USER_PORT, "C64 user port", tag, owner, clock, "c64_user_port", __FILE__),
		device_slot_interface(mconfig, *this),
		m_write_cnt1(*this),
		m_write_sp1(*this),
		m_write_cnt2(*this),
		m_write_sp2(*this),
		m_write_flag2(*this),
		m_write_reset(*this)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void c64_user_port_device::device_start()
{
	m_card = dynamic_cast<device_c64_user_port_interface *>(get_card_device());

	// resolve callbacks
	m_write_cnt1.resolve_safe();
	m_write_sp1.resolve_safe();
	m_write_cnt2.resolve_safe();
	m_write_sp2.resolve_safe();
	m_write_flag2.resolve_safe();
	m_write_reset.resolve_safe();
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void c64_user_port_device::device_reset()
{
	if (get_card_device())
	{
		get_card_device()->reset();
	}
}


READ8_MEMBER( c64_user_port_device::pb_r ) { UINT8 data = 0xff; if (m_card != NULL) data = m_card->c64_pb_r(space, offset); return data; }
WRITE8_MEMBER( c64_user_port_device::pb_w ) { if (m_card != NULL) m_card->c64_pb_w(space, offset, data); }
READ_LINE_MEMBER( c64_user_port_device::pa2_r ) { int state = 1; if (m_card != NULL) state = m_card->c64_pa2_r(); return state; }
WRITE_LINE_MEMBER( c64_user_port_device::pa2_w ) { if (m_card != NULL) m_card->c64_pa2_w(state); }
WRITE_LINE_MEMBER( c64_user_port_device::pc2_w ) { if (m_card != NULL) m_card->c64_pc2_w(state); }
WRITE_LINE_MEMBER( c64_user_port_device::atn_w ) { if (m_card != NULL) m_card->c64_atn_w(state); }
WRITE_LINE_MEMBER( c64_user_port_device::cnt1_w ) { if (m_card != NULL) m_card->c64_cnt1_w(state); }
WRITE_LINE_MEMBER( c64_user_port_device::sp1_w ) { if (m_card != NULL) m_card->c64_sp1_w(state); }
WRITE_LINE_MEMBER( c64_user_port_device::cnt2_w ) { if (m_card != NULL) m_card->c64_cnt2_w(state); }
WRITE_LINE_MEMBER( c64_user_port_device::sp2_w ) { if (m_card != NULL) m_card->c64_sp2_w(state); }
