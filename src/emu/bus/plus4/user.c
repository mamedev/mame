

/**********************************************************************

    Commodore Plus/4 User Port emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************/

#include "user.h"



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
	device_t(mconfig, PLUS4_USER_PORT, "User Port", tag, owner, clock, "plus4_user_port", __FILE__),
	device_slot_interface(mconfig, *this),
	m_4_handler(*this),
	m_5_handler(*this),
	m_6_handler(*this),
	m_7_handler(*this),
	m_8_handler(*this),
	m_b_handler(*this),
	m_c_handler(*this),
	m_f_handler(*this),
	m_h_handler(*this),
	m_j_handler(*this),
	m_k_handler(*this),
	m_l_handler(*this)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void plus4_user_port_device::device_start()
{
	m_cart = dynamic_cast<device_plus4_user_port_interface *>(get_card_device());

	// resolve callbacks
	m_4_handler.resolve_safe();
	m_5_handler.resolve_safe();
	m_6_handler.resolve_safe();
	m_7_handler.resolve_safe();
	m_8_handler.resolve_safe();
	m_b_handler.resolve_safe();
	m_c_handler.resolve_safe();
	m_f_handler.resolve_safe();
	m_h_handler.resolve_safe();
	m_j_handler.resolve_safe();
	m_k_handler.resolve_safe();
	m_l_handler.resolve_safe();

	// pull up
	m_4_handler(1);
	m_5_handler(1);
	m_6_handler(1);
	m_7_handler(1);
	m_8_handler(1);
	m_b_handler(1);
	m_c_handler(1);
	m_f_handler(1);
	m_h_handler(1);
	m_j_handler(1);
	m_k_handler(1);
	m_l_handler(1);
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void plus4_user_port_device::device_reset()
{
	if (get_card_device())
	{
		get_card_device()->reset();
	}
}


WRITE_LINE_MEMBER( plus4_user_port_device::write_4 ) { if (m_cart != NULL) m_cart->write_4(state); }
WRITE_LINE_MEMBER( plus4_user_port_device::write_5 ) { if (m_cart != NULL) m_cart->write_5(state); }
WRITE_LINE_MEMBER( plus4_user_port_device::write_6 ) { if (m_cart != NULL) m_cart->write_6(state); }
WRITE_LINE_MEMBER( plus4_user_port_device::write_7 ) { if (m_cart != NULL) m_cart->write_7(state); }
WRITE_LINE_MEMBER( plus4_user_port_device::write_8 ) { if (m_cart != NULL) m_cart->write_8(state); }
WRITE_LINE_MEMBER( plus4_user_port_device::write_9 ) { if (m_cart != NULL) m_cart->write_9(state); }
WRITE_LINE_MEMBER( plus4_user_port_device::write_b ) { if (m_cart != NULL) m_cart->write_b(state); }
WRITE_LINE_MEMBER( plus4_user_port_device::write_d ) { if (m_cart != NULL) m_cart->write_d(state); }
WRITE_LINE_MEMBER( plus4_user_port_device::write_e ) { if (m_cart != NULL) m_cart->write_e(state); }
WRITE_LINE_MEMBER( plus4_user_port_device::write_f ) { if (m_cart != NULL) m_cart->write_f(state); }
WRITE_LINE_MEMBER( plus4_user_port_device::write_j ) { if (m_cart != NULL) m_cart->write_j(state); }
WRITE_LINE_MEMBER( plus4_user_port_device::write_k ) { if (m_cart != NULL) m_cart->write_k(state); }
WRITE_LINE_MEMBER( plus4_user_port_device::write_m ) { if (m_cart != NULL) m_cart->write_m(state); }



//-------------------------------------------------
//  SLOT_INTERFACE( plus4_user_port_cards )
//-------------------------------------------------

// slot devices
#include "diag264_lb_user.h"

SLOT_INTERFACE_START( plus4_user_port_cards )
	SLOT_INTERFACE("diag264", DIAG264_USER_PORT_LOOPBACK)
SLOT_INTERFACE_END
