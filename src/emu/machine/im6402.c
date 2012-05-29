/***************************************************************************

    Intersil IM6402 Universal Asynchronous Receiver/Transmitter emulation

    Copyright the MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

***************************************************************************/

#include "im6402.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type IM6402 = &device_creator<im6402_device>;



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define LOG 1



//**************************************************************************
//  INLINE HELPERS
//**************************************************************************

//-------------------------------------------------
//  set_dr -
//-------------------------------------------------

inline void im6402_device::set_dr(int state)
{
	m_dr = state;

	m_out_dr_func(state);
}


//-------------------------------------------------
//  set_tbre -
//-------------------------------------------------

inline void im6402_device::set_tbre(int state)
{
	m_tbre = state;

	m_out_tbre_func(state);
}


//-------------------------------------------------
//  set_tre -
//-------------------------------------------------

inline void im6402_device::set_tre(int state)
{
	m_tre = state;

	m_out_tre_func(state);
}


//-------------------------------------------------
//  receive -
//-------------------------------------------------

inline void im6402_device::receive()
{
	int bit = 1;

	if (m_in_rri_func.isnull())
	{
		bit = get_in_data_bit();
	}
	else
	{
		bit = m_in_rri_func();
	}

//	if (LOG) logerror("IM6402 '%s' Receive Bit %u\n", tag(), bit);
	
	receive_register_update_bit(bit);

	if (is_receive_register_full())
	{
		receive_register_extract();
		m_rbr = get_received_char();
		
		if (LOG) logerror("IM6402 '%s' Receive Data %02x\n", tag(), m_rbr);

		if (m_dr)
		{
			m_oe = 1;
		}

		set_dr(ASSERT_LINE);
	}
}


//-------------------------------------------------
//  transmit -
//-------------------------------------------------

inline void im6402_device::transmit()
{
	if (is_transmit_register_empty() && !m_tbre)
	{
		if (LOG) logerror("IM6402 '%s' Transmit Data %02x\n", tag(), m_tbr);

		transmit_register_setup(m_tbr);

		set_tbre(ASSERT_LINE);
		set_tre(CLEAR_LINE);
	}

	if (!is_transmit_register_empty())
	{
		int bit = transmit_register_send_bit();

		if (LOG) logerror("IM6402 '%s' Transmit Bit %u\n", tag(), bit);

		m_out_tro_func(bit);

		if (is_transmit_register_empty())
		{
			set_tre(ASSERT_LINE);
		}
	}
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  im6402_device - constructor
//-------------------------------------------------

im6402_device::im6402_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
    : device_t(mconfig, IM6402, "Intersil IM6402", tag, owner, clock),
	  device_serial_interface(mconfig, *this)
{
}


//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void im6402_device::device_config_complete()
{
	// inherit a copy of the static data
	const im6402_interface *intf = reinterpret_cast<const im6402_interface *>(static_config());
	if (intf != NULL)
		*static_cast<im6402_interface *>(this) = *intf;

	// or initialize to defaults if none provided
	else
	{
		memset(&m_in_rri_cb, 0, sizeof(m_in_rri_cb));
		memset(&m_out_tro_cb, 0, sizeof(m_out_tro_cb));
		memset(&m_out_dr_cb, 0, sizeof(m_out_dr_cb));
		memset(&m_out_tbre_cb, 0, sizeof(m_out_tbre_cb));
		memset(&m_out_tre_cb, 0, sizeof(m_out_tre_cb));
	}
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void im6402_device::device_start()
{
	// resolve callbacks
	m_in_rri_func.resolve(m_in_rri_cb, *this);
	m_out_tro_func.resolve(m_out_tro_cb, *this);
	m_out_dr_func.resolve(m_out_dr_cb, *this);
	m_out_tbre_func.resolve(m_out_tbre_cb, *this);
	m_out_tre_func.resolve(m_out_tre_cb, *this);

	// create the timers
	if (m_rrc > 0)
	{
		m_rx_timer = timer_alloc(TIMER_RX);
		m_rx_timer->adjust(attotime::zero, 0, attotime::from_hz(m_rrc));
	}

	if (m_trc > 0)
	{
		m_tx_timer = timer_alloc(TIMER_TX);
		m_tx_timer->adjust(attotime::zero, 0, attotime::from_hz(m_trc));
	}

	// state saving
	save_item(NAME(m_dr));
	save_item(NAME(m_tbre));
	save_item(NAME(m_tre));
	save_item(NAME(m_pe));
	save_item(NAME(m_fe));
	save_item(NAME(m_oe));
	save_item(NAME(m_cls1));
	save_item(NAME(m_cls2));
	save_item(NAME(m_sbs));
	save_item(NAME(m_sfd));
	save_item(NAME(m_epe));
	save_item(NAME(m_pi));
	save_item(NAME(m_rbr));
	save_item(NAME(m_rrc_count));
	save_item(NAME(m_tbr));
	save_item(NAME(m_trc_count));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void im6402_device::device_reset()
{
	transmit_register_reset();
	receive_register_reset();

	m_rrc_count = 0;
	m_trc_count = 0;

	m_rbr = 0;
	m_pe = 0;
	m_fe = 0;
	m_oe = 0;

	set_dr(CLEAR_LINE);
	set_tbre(ASSERT_LINE);
	set_tre(ASSERT_LINE);
}


//-------------------------------------------------
//  device_timer - handler timer events
//-------------------------------------------------

void im6402_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch (id)
	{
	case TIMER_RX:
		rrc_w(1);
		break;

	case TIMER_TX:
		trc_w(1);
		break;
	}
}


//-------------------------------------------------
//  input_callback -
//-------------------------------------------------

void im6402_device::input_callback(UINT8 state)
{
}


//-------------------------------------------------
//  read - receiver buffer register read
//-------------------------------------------------

READ8_MEMBER( im6402_device::read )
{
	return m_rbr;
}


//-------------------------------------------------
//  write - transmitter buffer register write
//-------------------------------------------------

WRITE8_MEMBER( im6402_device::write )
{
	if (LOG) logerror("IM6402 '%s' Transmit Buffer Register %02x\n", tag(), data);

	m_tbr = data;

	set_tbre(CLEAR_LINE);
}


//-------------------------------------------------
//  rrc_w - receiver register clock
//-------------------------------------------------

WRITE_LINE_MEMBER( im6402_device::rrc_w )
{
	if (state)
	{
		m_rrc_count++;

		if (m_rrc_count == 16)
		{
			receive();
			m_rrc_count = 0;
		}
	}
}


//-------------------------------------------------
//  trc_w - transmitter register clock
//-------------------------------------------------

WRITE_LINE_MEMBER( im6402_device::trc_w )
{
	if (state)
	{
		m_trc_count++;

		if (m_trc_count == 16)
		{
			transmit();
			m_trc_count = 0;
		}
	}
}


//-------------------------------------------------
//  dr_r - data received
//-------------------------------------------------

READ_LINE_MEMBER( im6402_device::dr_r )
{
	return m_dr;
}


//-------------------------------------------------
//  tbre_r - transmitter buffer register empty
//-------------------------------------------------

READ_LINE_MEMBER( im6402_device::tbre_r )
{
	return m_tbre;
}


//-------------------------------------------------
//  tre_r - transmitter register empty
//-------------------------------------------------

READ_LINE_MEMBER( im6402_device::tre_r )
{
	return m_tre;
}


//-------------------------------------------------
//  pe_r - parity error
//-------------------------------------------------

READ_LINE_MEMBER( im6402_device::pe_r )
{
	return m_pe;
}


//-------------------------------------------------
//  fe_r - framing error
//-------------------------------------------------

READ_LINE_MEMBER( im6402_device::fe_r )
{
	return m_fe;
}


//-------------------------------------------------
//  oe_r - overrun error
//-------------------------------------------------

READ_LINE_MEMBER( im6402_device::oe_r )
{
	return m_oe;
}


//-------------------------------------------------
//  rrd_w - receiver register disable
//-------------------------------------------------

WRITE_LINE_MEMBER( im6402_device::rrd_w )
{
}


//-------------------------------------------------
//  sfd_w - status flags disable
//-------------------------------------------------

WRITE_LINE_MEMBER( im6402_device::sfd_w )
{
}


//-------------------------------------------------
//  drr_w - data received reset
//-------------------------------------------------

WRITE_LINE_MEMBER( im6402_device::drr_w )
{
	if (state)
	{
		set_dr(CLEAR_LINE);
	}
}


//-------------------------------------------------
//  mr_w - master reset
//-------------------------------------------------

WRITE_LINE_MEMBER( im6402_device::mr_w )
{
	if (state)
	{
		device_reset();
	}
}


//-------------------------------------------------
//  crl_w - control register load
//-------------------------------------------------

WRITE_LINE_MEMBER( im6402_device::crl_w )
{
	if (state)
	{
		if (LOG) logerror("IM6402 '%s' Control Register Load\n", tag());

		int word_length = 5 + ((m_cls2 << 1) | m_cls1);
		int stop_bits = 1 + (m_sbs ? ((word_length == 5) ? 0.5 : 1) : 0);
		int parity_code;

		if (m_pi) parity_code = SERIAL_PARITY_NONE;
		else if (m_epe) parity_code = SERIAL_PARITY_EVEN;
		else parity_code = SERIAL_PARITY_ODD;

		set_data_frame(word_length, stop_bits, parity_code);
	}
}


//-------------------------------------------------
//  pi_w - parity inhibit
//-------------------------------------------------

WRITE_LINE_MEMBER( im6402_device::pi_w )
{
	if (LOG) logerror("IM6402 '%s' Parity Inhibit %u\n", tag(), state);

	m_pi = state;
}


//-------------------------------------------------
//  sbs_w - stop bit select
//-------------------------------------------------

WRITE_LINE_MEMBER( im6402_device::sbs_w )
{
	if (LOG) logerror("IM6402 '%s' Stop Bit Select %u\n", tag(), state);
	
	m_sbs = state;
}


//-------------------------------------------------
//  cls1_w - character length select 1
//-------------------------------------------------

WRITE_LINE_MEMBER( im6402_device::cls1_w )
{
	if (LOG) logerror("IM6402 '%s' Character Length Select 1 %u\n", tag(), state);

	m_cls1 = state;
}


//-------------------------------------------------
//  cls2_w - character length select 2
//-------------------------------------------------

WRITE_LINE_MEMBER( im6402_device::cls2_w )
{
	if (LOG) logerror("IM6402 '%s' Character Length Select 2 %u\n", tag(), state);
	
	m_cls2 = state;
}


//-------------------------------------------------
//  epe_w - even parity enable
//-------------------------------------------------

WRITE_LINE_MEMBER( im6402_device::epe_w )
{
	if (LOG) logerror("IM6402 '%s' Even Parity Enable %u\n", tag(), state);

	m_epe = state;
}
