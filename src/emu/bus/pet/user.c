// license:BSD-3-Clause
// copyright-holders:smf
/**********************************************************************

    Commodore PET User Port emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************/

#include "user.h"



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

const device_type PET_USER_PORT = &device_creator<pet_user_port_device>;



//**************************************************************************
//  CARD INTERFACE
//**************************************************************************

//-------------------------------------------------
//  device_pet_user_port_interface - constructor
//-------------------------------------------------

device_pet_user_port_interface::device_pet_user_port_interface(const machine_config &mconfig, device_t &device)
	: device_slot_card_interface(mconfig,device)
{
	m_slot = dynamic_cast<pet_user_port_device *>(device.owner());
}


//-------------------------------------------------
//  ~device_pet_user_port_interface - destructor
//-------------------------------------------------

device_pet_user_port_interface::~device_pet_user_port_interface()
{
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  pet_user_port_device - constructor
//-------------------------------------------------

pet_user_port_device::pet_user_port_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, PET_USER_PORT, "VIC-20 user port", tag, owner, clock, "pet_user_port", __FILE__),
	device_slot_interface(mconfig, *this),
	m_3_handler(*this),
	m_4_handler(*this),
	m_5_handler(*this),
	m_6_handler(*this),
	m_7_handler(*this),
	m_8_handler(*this),
	m_9_handler(*this),
	m_b_handler(*this),
	m_c_handler(*this),
	m_d_handler(*this),
	m_e_handler(*this),
	m_f_handler(*this),
	m_h_handler(*this),
	m_j_handler(*this),
	m_k_handler(*this),
	m_l_handler(*this),
	m_m_handler(*this)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void pet_user_port_device::device_start()
{
	m_card = dynamic_cast<device_pet_user_port_interface *>(get_card_device());

	// resolve callbacks
	m_3_handler.resolve_safe();
	m_4_handler.resolve_safe();
	m_5_handler.resolve_safe();
	m_6_handler.resolve_safe();
	m_7_handler.resolve_safe();
	m_8_handler.resolve_safe();
	m_9_handler.resolve_safe();
	m_b_handler.resolve_safe();
	m_c_handler.resolve_safe();
	m_d_handler.resolve_safe();
	m_e_handler.resolve_safe();
	m_f_handler.resolve_safe();
	m_h_handler.resolve_safe();
	m_j_handler.resolve_safe();
	m_k_handler.resolve_safe();
	m_l_handler.resolve_safe();
	m_m_handler.resolve_safe();

	// pull up
	m_3_handler(1);
	m_4_handler(1);
	m_5_handler(1);
	m_6_handler(1);
	m_7_handler(1);
	m_8_handler(1);
	m_9_handler(1);
	m_b_handler(1);
	m_c_handler(1);
	m_d_handler(1);
	m_e_handler(1);
	m_f_handler(1);
	m_h_handler(1);
	m_j_handler(1);
	m_k_handler(1);
	m_l_handler(1);
	m_m_handler(1);
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void pet_user_port_device::device_reset()
{
	if (get_card_device())
	{
		get_card_device()->reset();
	}
}


WRITE_LINE_MEMBER( pet_user_port_device::write_3 ) { if (m_card != NULL) m_card->input_3(state); }
WRITE_LINE_MEMBER( pet_user_port_device::write_4 ) { if (m_card != NULL) m_card->input_4(state); }
WRITE_LINE_MEMBER( pet_user_port_device::write_5 ) { if (m_card != NULL) m_card->input_5(state); }
WRITE_LINE_MEMBER( pet_user_port_device::write_6 ) { if (m_card != NULL) m_card->input_6(state); }
WRITE_LINE_MEMBER( pet_user_port_device::write_7 ) { if (m_card != NULL) m_card->input_7(state); }
WRITE_LINE_MEMBER( pet_user_port_device::write_8 ) { if (m_card != NULL) m_card->input_8(state); }
WRITE_LINE_MEMBER( pet_user_port_device::write_9 ) { if (m_card != NULL) m_card->input_9(state); }
WRITE_LINE_MEMBER( pet_user_port_device::write_b ) { if (m_card != NULL) m_card->input_b(state); }
WRITE_LINE_MEMBER( pet_user_port_device::write_c ) { if (m_card != NULL) m_card->input_c(state); }
WRITE_LINE_MEMBER( pet_user_port_device::write_d ) { if (m_card != NULL) m_card->input_d(state); }
WRITE_LINE_MEMBER( pet_user_port_device::write_e ) { if (m_card != NULL) m_card->input_e(state); }
WRITE_LINE_MEMBER( pet_user_port_device::write_f ) { if (m_card != NULL) m_card->input_f(state); }
WRITE_LINE_MEMBER( pet_user_port_device::write_h ) { if (m_card != NULL) m_card->input_h(state); }
WRITE_LINE_MEMBER( pet_user_port_device::write_j ) { if (m_card != NULL) m_card->input_j(state); }
WRITE_LINE_MEMBER( pet_user_port_device::write_k ) { if (m_card != NULL) m_card->input_k(state); }
WRITE_LINE_MEMBER( pet_user_port_device::write_l ) { if (m_card != NULL) m_card->input_l(state); }
WRITE_LINE_MEMBER( pet_user_port_device::write_m ) { if (m_card != NULL) m_card->input_m(state); }





//-------------------------------------------------
//  SLOT_INTERFACE( pet_user_port_cards )
//-------------------------------------------------

#include "petuja.h"

SLOT_INTERFACE_START( pet_user_port_cards )
	SLOT_INTERFACE("petuja", PET_USERPORT_JOYSTICK_ADAPTER)
SLOT_INTERFACE_END
