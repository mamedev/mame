/**********************************************************************

    NEC uPD7201 Multiprotocol Serial Communications Controller

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************/

/*

    TODO:

    - this is a clone of Intel 8274?
    - everything

*/

#include "emu.h"
#include "upd7201.h"


// device type definition
const device_type UPD7201 = &device_creator<upd7201_device>;


//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define LOG 0


enum
{
	CHANNEL_A = 0,
	CHANNEL_B
};



//**************************************************************************
//  INLINE HELPERS
//**************************************************************************

//-------------------------------------------------
//  receive -
//-------------------------------------------------

inline void upd7201_device::receive(int channel)
{
}


//-------------------------------------------------
//  transmit -
//-------------------------------------------------

inline void upd7201_device::transmit(int channel)
{
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  upd7201_device - constructor
//-------------------------------------------------

upd7201_device::upd7201_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, UPD7201, "UPD7201", tag, owner, clock)
{
}


//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void upd7201_device::device_config_complete()
{
	// inherit a copy of the static data
	const upd7201_interface *intf = reinterpret_cast<const upd7201_interface *>(static_config());
	if (intf != NULL)
		*static_cast<upd7201_interface *>(this) = *intf;

	// or initialize to defaults if none provided
	else
	{
		memset(&m_out_int_cb, 0, sizeof(m_out_int_cb));
	}
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void upd7201_device::device_start()
{
	// resolve callbacks
	m_out_int_func.resolve(m_out_int_cb, *this);
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void upd7201_device::device_reset()
{
}


//-------------------------------------------------
//  device_timer - handler timer events
//-------------------------------------------------

void upd7201_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch (id)
	{
	case TIMER_RX_A:
		receive(CHANNEL_A);
		break;

	case TIMER_TX_A:
		transmit(CHANNEL_A);
		break;

	case TIMER_RX_B:
		receive(CHANNEL_B);
		break;

	case TIMER_TX_B:
		transmit(CHANNEL_B);
		break;
	}
}


//-------------------------------------------------
//  cd_ba_r -
//-------------------------------------------------

READ8_MEMBER( upd7201_device::cd_ba_r )
{
	return 0;
}


//-------------------------------------------------
//  cd_ba_w -
//-------------------------------------------------

WRITE8_MEMBER( upd7201_device::cd_ba_w )
{
}


//-------------------------------------------------
//  ba_cd_r -
//-------------------------------------------------

READ8_MEMBER( upd7201_device::ba_cd_r )
{
	return 0;
}


//-------------------------------------------------
//  ba_cd_w -
//-------------------------------------------------

WRITE8_MEMBER( upd7201_device::ba_cd_w )
{
}


//-------------------------------------------------
//  intak_r -
//-------------------------------------------------

READ8_MEMBER( upd7201_device::intak_r )
{
	return 0;
}


//-------------------------------------------------
//  synca_w -
//-------------------------------------------------

WRITE_LINE_MEMBER( upd7201_device::synca_w )
{
}


//-------------------------------------------------
//  syncb_w -
//-------------------------------------------------

WRITE_LINE_MEMBER( upd7201_device::syncb_w )
{
}


//-------------------------------------------------
//  ctsa_w -
//-------------------------------------------------

WRITE_LINE_MEMBER( upd7201_device::ctsa_w )
{
}


//-------------------------------------------------
//  ctsb_w -
//-------------------------------------------------

WRITE_LINE_MEMBER( upd7201_device::ctsb_w )
{
}


//-------------------------------------------------
//  dtra_r -
//-------------------------------------------------

READ_LINE_MEMBER( upd7201_device::dtra_r )
{
	return 0;
}


//-------------------------------------------------
//  dtrb_r -
//-------------------------------------------------

READ_LINE_MEMBER( upd7201_device::dtrb_r )
{
	return 0;
}


//-------------------------------------------------
//  hai_w -
//-------------------------------------------------

WRITE_LINE_MEMBER( upd7201_device::hai_w )
{
}


//-------------------------------------------------
//  rxda_w -
//-------------------------------------------------

WRITE_LINE_MEMBER( upd7201_device::rxda_w )
{
}


//-------------------------------------------------
//  txda_r -
//-------------------------------------------------

READ_LINE_MEMBER( upd7201_device::txda_r )
{
	return 0;
}


//-------------------------------------------------
//  rxdb_w -
//-------------------------------------------------

WRITE_LINE_MEMBER( upd7201_device::rxdb_w )
{
}


//-------------------------------------------------
//  txdb_r -
//-------------------------------------------------

READ_LINE_MEMBER( upd7201_device::txdb_r )
{
	return 0;
}


//-------------------------------------------------
//  rxca_w -
//-------------------------------------------------

WRITE_LINE_MEMBER( upd7201_device::rxca_w )
{
}


//-------------------------------------------------
//  rxcb_w -
//-------------------------------------------------

WRITE_LINE_MEMBER( upd7201_device::rxcb_w )
{
}


//-------------------------------------------------
//  txca_w -
//-------------------------------------------------

WRITE_LINE_MEMBER( upd7201_device::txca_w )
{
}


//-------------------------------------------------
//  txcb_w -
//-------------------------------------------------

WRITE_LINE_MEMBER( upd7201_device::txcb_w )
{
}
