// license:BSD-3-Clause
// copyright-holders:Sergey Svishchev
/***************************************************************************

	DEC DL11-type SLU (serial line unit).

	Frame format is not software-configurable; hardcoded to 8N1 for now.

	http://www.ibiblio.org/pub/academic/computer-science/history/pdp-11/hardware/micronotes/numerical/micronote33.txt

***************************************************************************/

#include "emu.h"
#include "dl11.h"


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// device type definition
DEFINE_DEVICE_TYPE(DL11, dl11_device, "dl11", "DEC DL11-type SLU")


//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

enum
{
	DLRCSR = 0,
	DLRBUF,
	DLTCSR,
	DLTBUF
};

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  dl11_device - constructor
//-------------------------------------------------

dl11_device::dl11_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, DL11, tag, owner, clock)
	, device_serial_interface(mconfig, *this)
	, device_z80daisy_interface(mconfig, *this)
	, m_write_txd(*this)
	, m_write_rxrdy(*this)
	, m_write_txrdy(*this)
	, m_rxc(0)
	, m_txc(0)
	, m_rxvec(0)
	, m_txvec(0)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void dl11_device::device_start()
{
	// resolve callbacks
	m_write_txd.resolve_safe();
	m_write_rxrdy.resolve_safe();
	m_write_txrdy.resolve_safe();

	// save state
	save_item(NAME(m_rcsr));
	save_item(NAME(m_rbuf));
	save_item(NAME(m_tcsr));
	save_item(NAME(m_tbuf));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void dl11_device::device_reset()
{
	set_data_frame(1, 8, PARITY_NONE, STOP_BITS_1);

	// create the timers
	if (m_rxc > 0)
		set_rcv_rate(m_rxc);

	if (m_txc > 0)
		set_tra_rate(m_txc);

	m_rcsr = m_rbuf = m_tbuf = 0;
	m_tcsr = CSR_DONE;
	m_rxrdy = m_txrdy = CLEAR_LINE;

	m_write_txd(1);
	m_write_rxrdy(m_rxrdy);
	m_write_txrdy(m_txrdy);
}

int dl11_device::z80daisy_irq_state()
{
	if (m_rxrdy == ASSERT_LINE || m_txrdy == ASSERT_LINE)
		return Z80_DAISY_INT;
	else
		return 0;
}

int dl11_device::z80daisy_irq_ack()
{
	int vec = -1;

	if (m_rxrdy == ASSERT_LINE)
	{
		m_rxrdy = CLEAR_LINE;
		vec = m_rxvec;
	}
	else if (m_txrdy == ASSERT_LINE)
	{
		m_txrdy = CLEAR_LINE;
		vec = m_txvec;
	}

	return vec;
}

void dl11_device::z80daisy_irq_reti()
{
}


//-------------------------------------------------
//  tra_callback -
//-------------------------------------------------

void dl11_device::tra_callback()
{
	if (m_tcsr & DLTCSR_XBRK)
	{
		m_write_txd(0);
	}
	else if (!is_transmit_register_empty())
	{
		m_write_txd(transmit_register_get_data_bit());
	}
}


//-------------------------------------------------
//  tra_complete -
//-------------------------------------------------

void dl11_device::tra_complete()
{
	m_tcsr |= CSR_DONE;
	raise_virq(m_write_txrdy, m_tcsr, CSR_IE, m_txrdy);
}


//-------------------------------------------------
//  rcv_complete -
//-------------------------------------------------

void dl11_device::rcv_complete()
{
	receive_register_extract();
	if (is_receive_framing_error())
	{
		m_rbuf = DLRBUF_ERR | DLRBUF_RBRK;
	}
	else
	{
		m_rbuf = get_received_char();
	}
	if (is_receive_parity_error())
	{
		m_rbuf |= DLRBUF_ERR | DLRBUF_PERR;
	}
	if (m_rcsr & CSR_DONE)
	{
		m_rbuf |= DLRBUF_ERR | DLRBUF_OVR;
	}
	else
	{
		m_rcsr |= CSR_DONE;
	}
	raise_virq(m_write_rxrdy, m_rcsr, CSR_IE, m_rxrdy);
}


//-------------------------------------------------
//  read - register read
//-------------------------------------------------

uint16_t dl11_device::read(offs_t offset)
{
	uint16_t data = 0;

	switch (offset & 3)
	{
	case DLRCSR:
		data = m_rcsr & DLRCSR_RD;
		break;

	case DLRBUF:
		data = m_rbuf;
		m_rcsr &= ~CSR_DONE;
		clear_virq(m_write_rxrdy, m_rcsr, CSR_IE, m_rxrdy);
		break;

	case DLTCSR:
		data = m_tcsr & DLTCSR_RD;
		break;

	case DLTBUF:
		data = m_tbuf;
		break;
	}

	return data;
}


//-------------------------------------------------
//  write - register write
//-------------------------------------------------

void dl11_device::write(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	switch (offset & 3)
	{
	case DLRCSR:
		if ((data & CSR_IE) == 0)
		{
			clear_virq(m_write_rxrdy, 1, 1, m_rxrdy);
		}
		else if ((m_rcsr & (CSR_DONE + CSR_IE)) == CSR_DONE)
		{
			raise_virq(m_write_rxrdy, 1, 1, m_rxrdy);
		}
		m_rcsr = ((m_rcsr & ~DLRCSR_WR) | (data & DLRCSR_WR));
		break;

	case DLRBUF:
		break;

	case DLTCSR:
		if ((data & CSR_IE) == 0)
		{
			clear_virq(m_write_txrdy, 1, 1, m_txrdy);
		}
		else if ((m_tcsr & (CSR_DONE + CSR_IE)) == CSR_DONE)
		{
			raise_virq(m_write_txrdy, 1, 1, m_txrdy);
		}
		m_tcsr = ((m_tcsr & ~DLTCSR_WR) | (data & DLTCSR_WR));
		break;

	case DLTBUF:
		m_tbuf = data;
		m_tcsr &= ~CSR_DONE;
		clear_virq(m_write_txrdy, m_tcsr, CSR_IE, m_txrdy);
		transmit_register_setup(data & 0xff);
		break;
	}
}


//-------------------------------------------------
//  rxrdy_r - receiver ready
//-------------------------------------------------

READ_LINE_MEMBER(dl11_device::rxrdy_r)
{
	return ((m_rcsr & (CSR_DONE | CSR_IE)) == (CSR_DONE | CSR_IE)) ? ASSERT_LINE : CLEAR_LINE;
}


//-------------------------------------------------
//  txrdy_r - transmitter empty
//-------------------------------------------------

READ_LINE_MEMBER(dl11_device::txrdy_r)
{
	return ((m_tcsr & (CSR_DONE | CSR_IE)) == (CSR_DONE | CSR_IE)) ? ASSERT_LINE : CLEAR_LINE;
}
