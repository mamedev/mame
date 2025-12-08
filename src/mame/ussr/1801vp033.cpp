// license:BSD-3-Clause
// copyright-holders:Sergey Svishchev
/**********************************************************************

    1801VP1-033 Gate Array emulation

    https://github.com/1801BM1/k1801/blob/master/033/doc/bpic.md

    Multifunction device; function is chosen at power-up time:

    BPIC - bidirectional 8-bit parallel port with interrupt support
           (for printers, paper tape readers and terminals)
    PIC - bidirectional 16-bit parallel port with interrupt support
    FDIC - bidirectional parallel-to-serial interface
           (for external floppy drive, clone of RX01 or RX02)

    Unimplemented: FDIC function and bidirectional mode of BPIC function

**********************************************************************/

#include "emu.h"
#include "1801vp033.h"

//#define VERBOSE (LOG_GENERAL)
#include "logmacro.h"


//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(K1801VP033, k1801vp033_device, "1801vp1_033", "1801VP1-033 parallel/serial interface")



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  k1801vp033_device - constructor
//-------------------------------------------------

k1801vp033_device::k1801vp033_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, K1801VP033, tag, owner, clock)
	, device_z80daisy_interface(mconfig, *this)
	, m_bpic_write_rxrdy(*this)
	, m_bpic_write_txrdy(*this)
	, m_bpic_write_reset(*this)
	, m_bpic_write_strobe(*this)
	, m_bpic_write_pd(*this)
	, m_pic_write_reqa(*this)
	, m_pic_write_reqb(*this)
	, m_pic_write_csr0(*this)
	, m_pic_write_csr1(*this)
	, m_pic_write_out(*this)
	, m_rxvec(0)
	, m_txvec(0)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void k1801vp033_device::device_start()
{
	m_rcsr = m_tcsr = m_rbuf = m_tbuf = 0;
	m_rxrdy = m_txrdy = CLEAR_LINE;

	m_bpic_write_strobe(1);

	// register for state saving
	save_item(NAME(m_rcsr));
	save_item(NAME(m_rbuf));
	save_item(NAME(m_tcsr));
	save_item(NAME(m_tbuf));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void k1801vp033_device::device_reset()
{
	m_tbuf = 0;
	m_txrdy = CLEAR_LINE;
	m_tcsr &= ~(CSR_IE | PICCSR_IEA | PICCSR_IEB);
	m_tcsr |= BPICCSR_DRQ;
}

//-------------------------------------------------
//  daisy chained interrupts
//-------------------------------------------------

int k1801vp033_device::z80daisy_irq_state()
{
	if (m_rxrdy == ASSERT_LINE || m_txrdy == ASSERT_LINE)
		return Z80_DAISY_INT;
	else
		return 0;
}

int k1801vp033_device::z80daisy_irq_ack()
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


//-------------------------------------------------
//  read - register read
//-------------------------------------------------

uint16_t k1801vp033_device::bpic_read(offs_t offset)
{
	uint16_t data = 0;

	switch (offset & 3)
	{
	case 0:
		data = m_tcsr & BPICCSR_RD;
		break;

	case 2:
		data = m_tbuf;
		break;
	}

	return data;
}


uint16_t k1801vp033_device::pic_read(offs_t offset)
{
	uint16_t data = 0;

	switch (offset & 3)
	{
	case 0:
		data = m_tcsr & PICCSR_RD;
		break;

	case 1:
		data = m_tbuf;
		break;

	case 2:
		data = m_rbuf;
		break;
	}

	return data;
}

//-------------------------------------------------
//  write - register write
//-------------------------------------------------

void k1801vp033_device::bpic_write(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	LOG("%s W %06o <- %06o & %06o\n", machine().describe_context(), 0177510 + (offset << 1), data, mem_mask);

	switch (offset & 3)
	{
	case 0:
		if ((data & CSR_IE) == 0)
		{
			clear_virq(m_bpic_write_txrdy, 1, 1, m_txrdy);
		}
		else if ((m_tcsr & (BPICCSR_DRQ + CSR_IE)) == BPICCSR_DRQ)
		{
			raise_virq(m_bpic_write_txrdy, 1, 1, m_txrdy);
		}
		m_tcsr = ((m_tcsr & ~BPICCSR_WR) | (data & BPICCSR_WR));
		m_bpic_write_reset(!BIT(m_tcsr, 14));
		break;

	case 2:
		m_tbuf = data & 0377;
		m_tcsr &= ~BPICCSR_DRQ;
		clear_virq(m_bpic_write_txrdy, m_tcsr, CSR_IE, m_txrdy);
		m_bpic_write_pd(m_tbuf);
		m_bpic_write_strobe(0);
		m_bpic_write_strobe(1);
		break;
	}
}

void k1801vp033_device::pic_write(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	LOG("%s W %06o <- %06o & %06o\n", machine().describe_context(), 0177510 + (offset << 1), data, mem_mask);

	switch (offset & 3)
	{
	case 0:
		m_tcsr = ((m_tcsr & ~PICCSR_WR) | (data & PICCSR_WR));
		m_pic_write_csr0(BIT(m_tcsr, 0));
		m_pic_write_csr1(BIT(m_tcsr, 1));
		break;

	case 1:
		COMBINE_DATA(&m_tbuf);
		m_pic_write_out(0, m_tbuf, mem_mask);
		break;
	}
}
