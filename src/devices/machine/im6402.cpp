// license:BSD-3-Clause
// copyright-holders:Curt Coder
/***************************************************************************

    Intersil IM6402 Universal Asynchronous Receiver/Transmitter emulation

***************************************************************************/

#include "im6402.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type IM6402 = &device_creator<im6402_device>;



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define LOG 0



//**************************************************************************
//  INLINE HELPERS
//**************************************************************************

//-------------------------------------------------
//  set_dr -
//-------------------------------------------------

inline void im6402_device::set_dr(int state)
{
	m_dr = state;

	m_write_dr(state);
}


//-------------------------------------------------
//  set_tbre -
//-------------------------------------------------

inline void im6402_device::set_tbre(int state)
{
	m_tbre = state;

	m_write_tbre(state);
}


//-------------------------------------------------
//  set_tre -
//-------------------------------------------------

inline void im6402_device::set_tre(int state)
{
	m_tre = state;

	m_write_tre(state);
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  im6402_device - constructor
//-------------------------------------------------

im6402_device::im6402_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, IM6402, "Intersil IM6402", tag, owner, clock, "im6402", __FILE__),
	device_serial_interface(mconfig, *this),
	m_write_tro(*this),
	m_write_dr(*this),
	m_write_tbre(*this),
	m_write_tre(*this),
	m_rrc_count(0),
	m_trc_count(0)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void im6402_device::device_start()
{
	// resolve callbacks
	m_write_tro.resolve_safe();
	m_write_dr.resolve_safe();
	m_write_tbre.resolve_safe();
	m_write_tre.resolve_safe();

	// create the timers
	if (m_rrc > 0)
	{
		set_rcv_rate(m_rrc/16);
	}

	if (m_trc > 0)
	{
		set_tra_rate(m_trc/16);
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
	receive_register_reset();
	transmit_register_reset();

	m_write_tro(1);

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
	device_serial_interface::device_timer(timer, id, param, ptr);
}


//-------------------------------------------------
//  tra_callback -
//-------------------------------------------------

void im6402_device::tra_callback()
{
	m_write_tro(transmit_register_get_data_bit());
}


//-------------------------------------------------
//  tra_complete -
//-------------------------------------------------

void im6402_device::tra_complete()
{
	if (!m_tbre)
	{
		if (LOG) logerror("IM6402 '%s' Transmit Data %02x\n", tag(), m_tbr);

		transmit_register_setup(m_tbr);

		set_tbre(ASSERT_LINE);
		set_tre(CLEAR_LINE);
	}
}


//-------------------------------------------------
//  rcv_callback -
//-------------------------------------------------

void im6402_device::rcv_callback()
{
}


//-------------------------------------------------
//  rcv_complete -
//-------------------------------------------------

void im6402_device::rcv_complete()
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


//-------------------------------------------------
//  write - transmitter buffer register write
//-------------------------------------------------

WRITE8_MEMBER( im6402_device::write )
{
	if (LOG) logerror("IM6402 '%s' Transmit Buffer Register %02x\n", tag(), data);

	m_tbr = data;

	if (is_transmit_register_empty())
	{
		if (LOG) logerror("IM6402 '%s' Transmit Data %02x\n", tag(), m_tbr);

		transmit_register_setup(m_tbr);

		set_tbre(ASSERT_LINE);
		set_tre(CLEAR_LINE);
	}
	else
	{
		set_tbre(CLEAR_LINE);
	}
}


//-------------------------------------------------
//  rrc_w - receiver register clock
//-------------------------------------------------

WRITE_LINE_MEMBER( im6402_device::rrc_w )
{
	if (state)
	{
		rx_clock_w(m_rrc_count < 8);
		m_rrc_count = (m_rrc_count + 1) & 15;
	}
}


//-------------------------------------------------
//  trc_w - transmitter register clock
//-------------------------------------------------

WRITE_LINE_MEMBER( im6402_device::trc_w )
{
	if (state)
	{
		tx_clock_w(m_trc_count < 8);
		m_trc_count = (m_trc_count + 1) & 15;
	}
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

		int data_bit_count = 5 + ((m_cls2 << 1) | m_cls1);
		stop_bits_t stop_bits = (m_sbs ? ((data_bit_count == 5) ? STOP_BITS_1_5 : STOP_BITS_2) : STOP_BITS_1);
		parity_t parity;

		if (m_pi) parity = PARITY_NONE;
		else if (m_epe) parity = PARITY_EVEN;
		else parity = PARITY_ODD;

		set_data_frame(1, data_bit_count, parity, stop_bits);
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

WRITE_LINE_MEMBER(im6402_device::write_rri)
{
	// HACK derive clock from data line as wangpckb sends bytes instantly to make up for mcs51 serial implementation
	receive_register_update_bit(state);
	rx_clock_w(1);
	rx_clock_w(0);
}
