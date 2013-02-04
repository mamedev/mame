/**********************************************************************

    Commodore PET User Port emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************/

#include "machine/petuser.h"



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
		device_t(mconfig, PET_USER_PORT, "PET user port", tag, owner, clock),
		device_slot_interface(mconfig, *this)
{
}


//-------------------------------------------------
//  pet_user_port_device - destructor
//-------------------------------------------------

pet_user_port_device::~pet_user_port_device()
{
}


//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void pet_user_port_device::device_config_complete()
{
	// inherit a copy of the static data
	const pet_user_port_interface *intf = reinterpret_cast<const pet_user_port_interface *>(static_config());
	if (intf != NULL)
	{
		*static_cast<pet_user_port_interface *>(this) = *intf;
	}

	// or initialize to defaults if none provided
	else
	{
		memset(&m_out_ca1_cb, 0, sizeof(m_out_ca1_cb));
		memset(&m_out_cb2_cb, 0, sizeof(m_out_cb2_cb));
	}
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void pet_user_port_device::device_start()
{
	m_cart = dynamic_cast<device_pet_user_port_interface *>(get_card_device());

	// resolve callbacks
	m_out_ca1_func.resolve(m_out_ca1_cb, *this);
	m_out_cb2_func.resolve(m_out_cb2_cb, *this);
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void pet_user_port_device::device_reset()
{
}


READ8_MEMBER( pet_user_port_device::pa_r ) { UINT8 data = 0xff; if (m_cart != NULL) data = m_cart->pet_pa_r(space, offset); return data; }
WRITE8_MEMBER( pet_user_port_device::pa_w ) { if (m_cart != NULL) m_cart->pet_pa_w(space, offset, data); }
READ_LINE_MEMBER( pet_user_port_device::ca1_r ) { int state = 1; if (m_cart != NULL) state = m_cart->pet_ca1_r(); return state; }
WRITE_LINE_MEMBER( pet_user_port_device::ca1_w ) { if (m_cart != NULL) m_cart->pet_ca1_w(state); }
READ_LINE_MEMBER( pet_user_port_device::cb2_r ) { int state = 1; if (m_cart != NULL) state = m_cart->pet_cb2_r(); return state; }
WRITE_LINE_MEMBER( pet_user_port_device::cb2_w ) { if (m_cart != NULL) m_cart->pet_cb2_w(state); }
