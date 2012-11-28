/**********************************************************************

    COMX-35 Expansion Slot emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************/

#include "emu.h"
#include "emuopts.h"
#include "machine/comxexp.h"


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

const device_type COMX_EXPANSION_SLOT = &device_creator<comx_expansion_slot_device>;


//**************************************************************************
//  DEVICE COMX_EXPANSION CARD INTERFACE
//**************************************************************************

//-------------------------------------------------
//  device_comx_expansion_card_interface - constructor
//-------------------------------------------------

device_comx_expansion_card_interface::device_comx_expansion_card_interface(const machine_config &mconfig, device_t &device)
	: device_slot_card_interface(mconfig, device),
		m_ds(1)
{
	m_slot = dynamic_cast<comx_expansion_slot_device *>(device.owner());
}


//-------------------------------------------------
//  ~device_comx_expansion_card_interface - destructor
//-------------------------------------------------

device_comx_expansion_card_interface::~device_comx_expansion_card_interface()
{
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  comx_expansion_slot_device - constructor
//-------------------------------------------------

comx_expansion_slot_device::comx_expansion_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
		device_t(mconfig, COMX_EXPANSION_SLOT, "COMX-35 expansion slot", tag, owner, clock),
		device_slot_interface(mconfig, *this)
{
}


//-------------------------------------------------
//  comx_expansion_slot_device - destructor
//-------------------------------------------------

comx_expansion_slot_device::~comx_expansion_slot_device()
{
}


//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void comx_expansion_slot_device::device_config_complete()
{
	// inherit a copy of the static data
	const comx_expansion_slot_interface *intf = reinterpret_cast<const comx_expansion_slot_interface *>(static_config());
	if (intf != NULL)
	{
		*static_cast<comx_expansion_slot_interface *>(this) = *intf;
	}

	// or initialize to defaults if none provided
	else
	{
		memset(&m_out_int_cb, 0, sizeof(m_out_int_cb));
		memset(&m_out_wait_cb, 0, sizeof(m_out_wait_cb));
		memset(&m_out_clear_cb, 0, sizeof(m_out_clear_cb));
	}
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void comx_expansion_slot_device::device_start()
{
	m_card = dynamic_cast<device_comx_expansion_card_interface *>(get_card_device());

	// resolve callbacks
	m_out_int_func.resolve(m_out_int_cb, *this);
	m_out_wait_func.resolve(m_out_wait_cb, *this);
	m_out_clear_func.resolve(m_out_clear_cb, *this);
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void comx_expansion_slot_device::device_reset()
{
}


//-------------------------------------------------
//  mrd_r - memory read
//-------------------------------------------------

UINT8 comx_expansion_slot_device::mrd_r(address_space &space, offs_t offset, int *extrom)
{
	UINT8 data = 0;

	if (m_card != NULL)
	{
		data = m_card->comx_mrd_r(space, offset, extrom);
	}

	return data;
}


//-------------------------------------------------
//  mwr_w - memory write
//-------------------------------------------------

void comx_expansion_slot_device::mwr_w(address_space &space, offs_t offset, UINT8 data)
{
	if (m_card != NULL)
	{
		m_card->comx_mwr_w(space, offset, data);
	}
}


//-------------------------------------------------
//  io_r - I/O read
//-------------------------------------------------

UINT8 comx_expansion_slot_device::io_r(address_space &space, offs_t offset)
{
	UINT8 data = 0;

	if (m_card != NULL)
	{
		data = m_card->comx_io_r(space, offset);
	}

	return data;
}


//-------------------------------------------------
//  sout_w - I/O write
//-------------------------------------------------

void comx_expansion_slot_device::io_w(address_space &space, offs_t offset, UINT8 data)
{
	if (m_card != NULL)
	{
		m_card->comx_io_w(space, offset, data);
	}
}


//-------------------------------------------------
//  ds_w - device select write
//-------------------------------------------------

WRITE_LINE_MEMBER( comx_expansion_slot_device::ds_w )
{
	if (m_card != NULL)
	{
		m_card->comx_ds_w(state);
	}
}


//-------------------------------------------------
//  q_w - Q write
//-------------------------------------------------

WRITE_LINE_MEMBER( comx_expansion_slot_device::q_w )
{
	if (m_card != NULL)
	{
		m_card->comx_q_w(state);
	}
}

READ_LINE_MEMBER( comx_expansion_slot_device::ef4_r )
{
	int state = CLEAR_LINE;

	if (m_card != NULL)
	{
		state = m_card->comx_ef4_r();
	}

	return state;
}

WRITE_LINE_MEMBER( comx_expansion_slot_device::int_w ) { m_out_int_func(state); }
WRITE_LINE_MEMBER( comx_expansion_slot_device::wait_w ) { m_out_wait_func(state); }
WRITE_LINE_MEMBER( comx_expansion_slot_device::clear_w ) { m_out_clear_func(state); }
