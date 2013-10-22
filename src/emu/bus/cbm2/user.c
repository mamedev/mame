// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Commodore CBM-II User Port emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************/

#include "user.h"



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

const device_type CBM2_USER_PORT = &device_creator<cbm2_user_port_device>;



//**************************************************************************
//  CARD INTERFACE
//**************************************************************************

//-------------------------------------------------
//  device_cbm2_user_port_interface - constructor
//-------------------------------------------------

device_cbm2_user_port_interface::device_cbm2_user_port_interface(const machine_config &mconfig, device_t &device)
	: device_slot_card_interface(mconfig,device)
{
	m_slot = dynamic_cast<cbm2_user_port_device *>(device.owner());
}


//-------------------------------------------------
//  ~device_cbm2_user_port_interface - destructor
//-------------------------------------------------

device_cbm2_user_port_interface::~device_cbm2_user_port_interface()
{
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  cbm2_user_port_device - constructor
//-------------------------------------------------

cbm2_user_port_device::cbm2_user_port_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
		device_t(mconfig, CBM2_USER_PORT, "CBM2 user port", tag, owner, clock, "cbm2_user_port", __FILE__),
		device_slot_interface(mconfig, *this)
{
}


//-------------------------------------------------
//  cbm2_user_port_device - destructor
//-------------------------------------------------

cbm2_user_port_device::~cbm2_user_port_device()
{
}


//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void cbm2_user_port_device::device_config_complete()
{
	// inherit a copy of the static data
	const cbm2_user_port_interface *intf = reinterpret_cast<const cbm2_user_port_interface *>(static_config());
	if (intf != NULL)
	{
		*static_cast<cbm2_user_port_interface *>(this) = *intf;
	}

	// or initialize to defaults if none provided
	else
	{
		memset(&m_out_irq_cb, 0, sizeof(m_out_irq_cb));
		memset(&m_out_sp_cb, 0, sizeof(m_out_sp_cb));
		memset(&m_out_cnt_cb, 0, sizeof(m_out_cnt_cb));
		memset(&m_out_flag_cb, 0, sizeof(m_out_flag_cb));
	}
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void cbm2_user_port_device::device_start()
{
	m_card = dynamic_cast<device_cbm2_user_port_interface *>(get_card_device());

	// resolve callbacks
	m_out_irq_func.resolve(m_out_irq_cb, *this);
	m_out_sp_func.resolve(m_out_sp_cb, *this);
	m_out_cnt_func.resolve(m_out_cnt_cb, *this);
	m_out_flag_func.resolve(m_out_flag_cb, *this);
}


READ8_MEMBER( cbm2_user_port_device::d1_r ) { UINT8 data = 0xff; if (m_card != NULL) data = m_card->cbm2_d1_r(space, offset); return data; }
WRITE8_MEMBER( cbm2_user_port_device::d1_w ) { if (m_card != NULL) m_card->cbm2_d1_w(space, offset, data); }
READ8_MEMBER( cbm2_user_port_device::d2_r ) { UINT8 data = 0xff; if (m_card != NULL) data = m_card->cbm2_d2_r(space, offset); return data; }
WRITE8_MEMBER( cbm2_user_port_device::d2_w ) { if (m_card != NULL) m_card->cbm2_d2_w(space, offset, data); }
READ_LINE_MEMBER( cbm2_user_port_device::pb2_r ) { return m_card ? m_card->cbm2_pb2_r() : 1; }
WRITE_LINE_MEMBER( cbm2_user_port_device::pb2_w ) { if (m_card != NULL) m_card->cbm2_pb2_w(state); }
READ_LINE_MEMBER( cbm2_user_port_device::pb3_r ) { return m_card ? m_card->cbm2_pb3_r() : 1; }
WRITE_LINE_MEMBER( cbm2_user_port_device::pb3_w ) { if (m_card != NULL) m_card->cbm2_pb3_w(state); }
WRITE_LINE_MEMBER( cbm2_user_port_device::pc_w ) { if (m_card != NULL) m_card->cbm2_pc_w(state); }
WRITE_LINE_MEMBER( cbm2_user_port_device::cnt_w ) { if (m_card != NULL) m_card->cbm2_cnt_w(state); }
WRITE_LINE_MEMBER( cbm2_user_port_device::sp_w ) { if (m_card != NULL) m_card->cbm2_sp_w(state); }


//-------------------------------------------------
//  SLOT_INTERFACE( cbm2_user_port_cards )
//-------------------------------------------------

SLOT_INTERFACE_START( cbm2_user_port_cards )
SLOT_INTERFACE_END
