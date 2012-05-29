/***************************************************************************

    Philips SCN2661 Enhanced Programmable Communications Interface emulation

    Copyright the MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

***************************************************************************/

#include "scn2661.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type SCN2661 = &device_creator<scn2661_device>;



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define LOG 1

enum
{
	REGISTER_HOLDING = 0,
	REGISTER_STATUS,
	REGISTER_SYNC = REGISTER_STATUS,
	REGISTER_MODE,
	REGISTER_COMMAND
};


//**************************************************************************
//  INLINE HELPERS
//**************************************************************************

//-------------------------------------------------
//  receive -
//-------------------------------------------------

inline void scn2661_device::receive()
{
}


//-------------------------------------------------
//  transmit -
//-------------------------------------------------

inline void scn2661_device::transmit()
{
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  scn2661_device - constructor
//-------------------------------------------------

scn2661_device::scn2661_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
    : device_t(mconfig, SCN2661, "SCN2661", tag, owner, clock),
	  device_serial_interface(mconfig, *this)
{
}


//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void scn2661_device::device_config_complete()
{
	// inherit a copy of the static data
	const scn2661_interface *intf = reinterpret_cast<const scn2661_interface *>(static_config());
	if (intf != NULL)
		*static_cast<scn2661_interface *>(this) = *intf;

	// or initialize to defaults if none provided
	else
	{
		memset(&m_in_rxd_cb, 0, sizeof(m_in_rxd_cb));
		memset(&m_out_txd_cb, 0, sizeof(m_out_txd_cb));
		memset(&m_out_rxrdy_cb, 0, sizeof(m_out_rxrdy_cb));
		memset(&m_out_txrdy_cb, 0, sizeof(m_out_txrdy_cb));
		memset(&m_out_rts_cb, 0, sizeof(m_out_rts_cb));
		memset(&m_out_dtr_cb, 0, sizeof(m_out_dtr_cb));
		memset(&m_out_txemt_dschg_cb, 0, sizeof(m_out_txemt_dschg_cb));
		memset(&m_out_bkdet_cb, 0, sizeof(m_out_bkdet_cb));
		memset(&m_out_xsync_cb, 0, sizeof(m_out_xsync_cb));
	}
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void scn2661_device::device_start()
{
	// resolve callbacks
	m_in_rxd_func.resolve(m_in_rxd_cb, *this);
	m_out_txd_func.resolve(m_out_txd_cb, *this);
	m_out_rxrdy_func.resolve(m_out_rxrdy_cb, *this);
	m_out_txrdy_func.resolve(m_out_txrdy_cb, *this);
	m_out_rts_func.resolve(m_out_rts_cb, *this);
	m_out_dtr_func.resolve(m_out_dtr_cb, *this);
	m_out_txemt_dschg_func.resolve(m_out_txemt_dschg_cb, *this);
	m_out_bkdet_func.resolve(m_out_bkdet_cb, *this);
	m_out_xsync_func.resolve(m_out_xsync_cb, *this);

	// create the timers
	if (m_rxc > 0)
	{
		m_rx_timer = timer_alloc(TIMER_RX);
		m_rx_timer->adjust(attotime::zero, 0, attotime::from_hz(m_rxc));
	}

	if (m_txc > 0)
	{
		m_tx_timer = timer_alloc(TIMER_TX);
		m_tx_timer->adjust(attotime::zero, 0, attotime::from_hz(m_txc));
	}
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void scn2661_device::device_reset()
{
	transmit_register_reset();
	receive_register_reset();
}


//-------------------------------------------------
//  device_timer - handler timer events
//-------------------------------------------------

void scn2661_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch (id)
	{
	case TIMER_RX:
		rxc_w(1);
		break;

	case TIMER_TX:
		txc_w(1);
		break;
	}
}


//-------------------------------------------------
//  input_callback -
//-------------------------------------------------

void scn2661_device::input_callback(UINT8 state)
{
}


//-------------------------------------------------
//  read - 
//-------------------------------------------------

READ8_MEMBER( scn2661_device::read )
{
	UINT8 data = 0;

	switch (offset & 0x03)
	{
	case REGISTER_HOLDING:
		data = m_rhr;
		break;

	case REGISTER_STATUS:
		data = m_sr;
		break;

	case REGISTER_MODE:
		break;

	case REGISTER_COMMAND:
		data = m_cr;
		break;
	}

	return data;
}


//-------------------------------------------------
//  write - transmitter buffer register write
//-------------------------------------------------

WRITE8_MEMBER( scn2661_device::write )
{
	switch (offset & 0x03)
	{
	case REGISTER_HOLDING:
		m_thr = data;
		break;

	case REGISTER_SYNC:
		break;

	case REGISTER_MODE:
		break;

	case REGISTER_COMMAND:
		m_cr = data;
		break;
	}
}


//-------------------------------------------------
//  rxc_w - receiver clock
//-------------------------------------------------

WRITE_LINE_MEMBER( scn2661_device::rxc_w )
{
}


//-------------------------------------------------
//  txc_w - transmitter clock
//-------------------------------------------------

WRITE_LINE_MEMBER( scn2661_device::txc_w )
{
}


//-------------------------------------------------
//  dsr_w - data set ready
//-------------------------------------------------

WRITE_LINE_MEMBER( scn2661_device::dsr_w )
{
}


//-------------------------------------------------
//  dcd_w - data carrier detect
//-------------------------------------------------

WRITE_LINE_MEMBER( scn2661_device::dcd_w )
{
}


//-------------------------------------------------
//  cts_w - clear to send
//-------------------------------------------------

WRITE_LINE_MEMBER( scn2661_device::cts_w )
{
}


//-------------------------------------------------
//  rxrdy_r - receiver ready
//-------------------------------------------------

READ_LINE_MEMBER( scn2661_device::rxrdy_r )
{
	return CLEAR_LINE;
}


//-------------------------------------------------
//  txemt_r - transmitter empty
//-------------------------------------------------

READ_LINE_MEMBER( scn2661_device::txemt_r )
{
	return CLEAR_LINE;
}
