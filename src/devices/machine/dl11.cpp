// license:BSD-3-Clause
// copyright-holders:Sergey Svishchev
/***************************************************************************

    DEC DL11-type SLU (serial line unit) and compatible devices.

    Frame format is not software-configurable; hardcoded to 8N1 for now.

    dl11_device implements all features of DL11-D Unibus device.

    1801VP1-065 is a single chip implementation used in Soviet clones:
    - error bits are in RCSR, not RBUF, and are not cleared by new data
    - on overflow, RBUF keeps first received byte
    - reading TBUF returns transmit interrupt vector
    - writing to RBUF does nothing
    - INIT does not set DONE in TCSR
    - supports RTS/CTS flow control
    - always sends 2 stop bits

    http://www.ibiblio.org/pub/academic/computer-science/history/pdp-11/hardware/micronotes/numerical/micronote33.txt
    bitsavers://pdf/dec/unibus/EK-DL11-TM-003_DL11_Asynchronous_Line_Interface_Manual_Sep75.pdf
    https://github.com/1801BM1/k1801/blob/master/065/

***************************************************************************/

#include "emu.h"
#include "dl11.h"


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// device type definition
DEFINE_DEVICE_TYPE(DL11, dl11_device, "dl11", "DEC DL11-D SLU")
DEFINE_DEVICE_TYPE(K1801VP065, k1801vp065_device, "1801vp1_065", "1801VP1-065")


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

dl11_device::dl11_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_serial_interface(mconfig, *this)
	, device_z80daisy_interface(mconfig, *this)
	, m_installed(false)
	, m_write_txd(*this)
	, m_write_rxrdy(*this)
	, m_write_txrdy(*this)
	, m_rxc(0)
	, m_txc(0)
	, m_rxvec(0)
	, m_txvec(0)
{
}

dl11_device::dl11_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: dl11_device(mconfig, DL11, tag, owner, clock)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void dl11_device::device_start()
{
	// save state
	save_item(NAME(m_rcsr));
	save_item(NAME(m_rbuf));
	save_item(NAME(m_tcsr));
	save_item(NAME(m_tbuf));

	m_installed = false;
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void dl11_device::device_reset()
{
	if (!m_installed)
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

		m_installed = true;
	}
	else
	{
		m_rbuf &= 255; // clear error bits
		m_rcsr &= ~(CSR_IE | CSR_DONE | DLRCSR_ACT);
		m_tcsr &= ~(DLTCSR_XBRK | DLTCSR_MAINT | CSR_IE);
		m_tcsr |= CSR_DONE;
		clear_virq(m_write_rxrdy, 1, 1, m_rxrdy);
		clear_virq(m_write_txrdy, 1, 1, m_txrdy);
	}
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
		transmit_register_get_data_bit();
		if (m_tcsr & DLTCSR_MAINT)
			rx_w(0);
		else
			m_write_txd(0);
	}
	else if (!is_transmit_register_empty())
	{
		if (m_tcsr & DLTCSR_MAINT)
			rx_w(transmit_register_get_data_bit());
		else
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
//  rcv_callback -
//-------------------------------------------------

void dl11_device::rx_w(int state)
{
	device_serial_interface::rx_w(state);
	if (is_receive_register_synchronized())
		m_rcsr |= DLRCSR_ACT;
}


//-------------------------------------------------
//  rcv_complete -
//-------------------------------------------------

void dl11_device::rcv_complete()
{
	receive_register_extract();
	m_rbuf = get_received_char();
	if (is_receive_framing_error())
	{
		m_rbuf |= DLRBUF_ERR | DLRBUF_RBRK;
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
	m_rcsr &= ~DLRCSR_ACT;
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
		if (!machine().side_effects_disabled())
		{
			m_rcsr &= ~CSR_DONE;
			clear_virq(m_write_rxrdy, m_rcsr, CSR_IE, m_rxrdy);
		}
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
		if (data & DLRCSR_RDRENB)
		{
			m_rcsr &= ~CSR_DONE;
			clear_virq(m_write_rxrdy, m_rcsr, CSR_IE, m_rxrdy);
		}
		break;

	case DLRBUF:
		m_rcsr &= ~CSR_DONE;
		clear_virq(m_write_rxrdy, m_rcsr, CSR_IE, m_rxrdy);
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
		if ((m_tcsr ^ data) & DLTCSR_XBRK)
		{
			if (data & DLTCSR_XBRK)
			{
				transmit_register_setup(0);
			}
			else
			{
				transmit_register_reset();
				if (m_tcsr & DLTCSR_MAINT)
					rx_w(1);
				else
					m_write_txd(1);
			}
		}
		m_tcsr = ((m_tcsr & ~DLTCSR_WR) | (data & DLTCSR_WR));
		break;

	case DLTBUF:
		m_tbuf = data & 255;
		m_tcsr &= ~CSR_DONE;
		clear_virq(m_write_txrdy, m_tcsr, CSR_IE, m_txrdy);
		transmit_register_setup((m_tcsr & DLTCSR_XBRK) ? 0 : m_tbuf);
		break;
	}
}


//-------------------------------------------------
//  rxrdy_r - receiver ready
//-------------------------------------------------

int dl11_device::rxrdy_r()
{
	return ((m_rcsr & (CSR_DONE | CSR_IE)) == (CSR_DONE | CSR_IE)) ? ASSERT_LINE : CLEAR_LINE;
}


//-------------------------------------------------
//  txrdy_r - transmitter empty
//-------------------------------------------------

int dl11_device::txrdy_r()
{
	return ((m_tcsr & (CSR_DONE | CSR_IE)) == (CSR_DONE | CSR_IE)) ? ASSERT_LINE : CLEAR_LINE;
}


//-------------------------------------------------
//  k1801vp065_device - constructor
//-------------------------------------------------

k1801vp065_device::k1801vp065_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: dl11_device(mconfig, K1801VP065, tag, owner, clock)
	, m_write_rts(*this)
{
}

void k1801vp065_device::device_reset()
{
	if (!m_installed)
	{
		dl11_device::device_reset();
		set_data_frame(1, 8, PARITY_NONE, STOP_BITS_2);
		m_write_rts(0);
	}
	else
	{
		m_rcsr &= ~(DLRCSR_RBRK | DLRCSR_OVR | DLRCSR_PERR | CSR_IE | CSR_DONE);
		m_tcsr &= ~(DLTCSR_XBRK | DLTCSR_MAINT | CSR_IE);
		clear_virq(m_write_rxrdy, 1, 1, m_rxrdy);
		clear_virq(m_write_txrdy, 1, 1, m_txrdy);
	}
}

void k1801vp065_device::rcv_complete()
{
	receive_register_extract();
	if (m_rcsr & CSR_DONE)
	{
		m_rcsr |= DLRCSR_OVR;
	}
	else
	{
		m_rbuf = get_received_char();
		m_rcsr |= CSR_DONE;
		if (is_receive_framing_error())
		{
			m_rcsr |= DLRCSR_RBRK;
		}
		if (is_receive_parity_error())
		{
			m_rcsr |= DLRCSR_PERR;
		}
	}
	raise_virq(m_write_rxrdy, m_rcsr, CSR_IE, m_rxrdy);
	m_write_rts(1);
}

//-------------------------------------------------
//  read - register read
//-------------------------------------------------

uint16_t k1801vp065_device::read(offs_t offset)
{
	uint16_t data = 0;

	switch (offset & 3)
	{
	case DLRCSR:
		data = m_rcsr & DLRCSR_RD;
		break;

	case DLRBUF:
		data = m_rbuf;
		if (!machine().side_effects_disabled())
		{
			m_rcsr &= ~(CSR_DONE | DLRCSR_PERR | DLRCSR_OVR | DLRCSR_RBRK);
			clear_virq(m_write_rxrdy, m_rcsr, CSR_IE, m_rxrdy);
			m_write_rts(0);
		}
		break;

	case DLTCSR:
		data = m_tcsr & DLTCSR_RD;
		break;

	case DLTBUF:
		data = m_rxvec;
		break;
	}

	return data;
}

//-------------------------------------------------
//  write - register write
//-------------------------------------------------

void k1801vp065_device::write(offs_t offset, uint16_t data)
{
	switch (offset & 3)
	{
	case DLRCSR:
		data &= ~DLRCSR_RDRENB;
		break;

	case DLRBUF:
		return;
	}
	dl11_device::write(offset, data);
}
