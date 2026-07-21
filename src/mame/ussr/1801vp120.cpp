// license:BSD-3-Clause
// copyright-holders:Sergey Svishchev
/**********************************************************************

    1801VP1-120 Gate Array emulation

    Inter-processor bridge for UKNC

    https://github.com/1801BM1/k1801/blob/master/120

**********************************************************************/

#include "emu.h"
#include "1801vp120.h"

//#define VERBOSE (LOG_GENERAL)
#include "logmacro.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(K1801VP120_PRI, k1801vp120_pri_device, "uknc_bridge_pri", "UKNC bridge maincpu")
DEFINE_DEVICE_TYPE(K1801VP120_SUB, k1801vp120_sub_device, "uknc_bridge_sub", "UKNC bridge subcpu")


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  k1801vp120_pri_device - constructor
//-------------------------------------------------

k1801vp120_pri_device::k1801vp120_pri_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, K1801VP120_PRI, tag, owner, clock)
	, device_z80daisy_interface(mconfig, *this)
	, m_write_virq(*this)
	, m_write_ack(*this)
	, m_write_out(*this)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void k1801vp120_pri_device::device_start()
{
	save_item(NAME(m_channel[0].rcsr));
	save_item(NAME(m_channel[0].rbuf));
	save_item(NAME(m_channel[0].tcsr));
	save_item(NAME(m_channel[1].rcsr));
	save_item(NAME(m_channel[1].rbuf));
	save_item(NAME(m_channel[1].tcsr));
	save_item(NAME(m_channel[2].tcsr));

	memset(m_channel, 0, sizeof(m_channel));
	m_channel[1].channel = 1;
	m_channel[2].channel = 2;
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void k1801vp120_pri_device::device_reset()
{
	m_channel[0].tcsr = m_channel[1].tcsr = m_channel[2].tcsr = CSR_DONE;
	m_channel[0].rcsr = m_channel[1].rcsr = m_channel[2].rcsr = 0;
}

//-------------------------------------------------
//  daisy chained interrupts
//-------------------------------------------------

int k1801vp120_pri_device::z80daisy_irq_state()
{
	if ((m_channel[0].rxrdy | m_channel[1].rxrdy | m_channel[0].txrdy | m_channel[1].txrdy | m_channel[2].txrdy) == ASSERT_LINE)
		return Z80_DAISY_INT;
	else
		return 0;
}

int k1801vp120_pri_device::z80daisy_irq_ack()
{
	int vec = -1;

	if (m_channel[0].rxrdy == ASSERT_LINE)
	{
		m_channel[0].rxrdy = CLEAR_LINE;
		vec = 060;
	}
	else if (m_channel[0].txrdy == ASSERT_LINE)
	{
		m_channel[0].txrdy = CLEAR_LINE;
		vec = 064;
	}
	else if (m_channel[1].rxrdy == ASSERT_LINE)
	{
		m_channel[1].rxrdy = CLEAR_LINE;
		vec = 0460;
	}
	else if (m_channel[1].txrdy == ASSERT_LINE)
	{
		m_channel[1].txrdy = CLEAR_LINE;
		vec = 0464;
	}
	else if (m_channel[2].txrdy == ASSERT_LINE)
	{
		m_channel[2].txrdy = CLEAR_LINE;
		vec = 0474;
	}

	return vec;
}


//-------------------------------------------------
//  read - register read
//-------------------------------------------------

uint16_t k1801vp120_pri_device::channel_read(struct xchannel *ch, offs_t offset)
{
	uint16_t data = 0;

	switch (offset)
	{
	case 0:
		data = ch->rcsr & PPxCSR_RD;
		break;

	case 1:
		data = ch->rbuf;
		if (!machine().side_effects_disabled())
		{
			ch->rcsr &= ~CSR_DONE;
			clear_virq(m_write_virq, ch->rcsr, CSR_IE, ch->rxrdy);
			if (ch->channel == 0 || ch->channel == 1)
				m_write_ack[ch->channel](ASSERT_LINE);
		}
		break;

	case 2:
		data = ch->tcsr & PPxCSR_RD;
		break;
	}

	if (ch->channel == 1)
	LOG("%s R ch %d %06o == %06o\n", machine().describe_context(), ch->channel, (offset << 1), data);

	return data;
}

//-------------------------------------------------
//  write - register write
//-------------------------------------------------

void k1801vp120_pri_device::channel_write(struct xchannel *ch, offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if (ch->channel == 1)
	LOG("%s W ch %d %06o <- %06o & %06o\n", machine().describe_context(), ch->channel, (offset << 1), data, mem_mask);

	switch (offset)
	{
	case 0:
		if ((data & CSR_IE) == 0)
			clear_virq(m_write_virq, 1, 1, ch->rxrdy);
		else if ((ch->rcsr & (CSR_DONE + CSR_IE)) == CSR_DONE)
			raise_virq(m_write_virq, 1, 1, ch->rxrdy);
		ch->rcsr = ((ch->rcsr & ~PPxCSR_WR) | (data & PPxCSR_WR));
		break;

	case 2:
		if ((data & CSR_IE) == 0)
			clear_virq(m_write_virq, 1, 1, ch->txrdy);
		else if ((ch->tcsr & (CSR_DONE + CSR_IE)) == CSR_DONE)
			raise_virq(m_write_virq, 1, 1, ch->txrdy);
		ch->tcsr = ((ch->tcsr & ~PPxCSR_WR) | (data & PPxCSR_WR));
		break;

	case 3:
		ch->tcsr &= ~CSR_DONE;
		clear_virq(m_write_virq, ch->tcsr, CSR_IE, ch->txrdy);
		m_write_out[ch->channel](data);
		break;
	}
}

//-------------------------------------------------
//  k1801vp120_sub_device - constructor
//-------------------------------------------------

k1801vp120_sub_device::k1801vp120_sub_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, K1801VP120_SUB, tag, owner, clock)
	, device_z80daisy_interface(mconfig, *this)
	, m_write_virq(*this)
	, m_write_ack(*this)
	, m_write_out(*this)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void k1801vp120_sub_device::device_start()
{
	save_item(NAME(m_channel[0].rbuf));
	save_item(NAME(m_channel[1].rbuf));
	save_item(NAME(m_channel[2].rbuf));
	save_item(NAME(m_rcsr));
	save_item(NAME(m_tcsr));

	m_reset = CLEAR_LINE;
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void k1801vp120_sub_device::device_reset()
{
	memset(m_channel, 0, sizeof(m_channel));

	m_rcsr = 0;
	m_tcsr = PPTCSR_1DONE | PPTCSR_0DONE;
}

//-------------------------------------------------
//  daisy chained interrupts
//-------------------------------------------------

int k1801vp120_sub_device::z80daisy_irq_state()
{
	if ((m_channel[0].rxrdy | m_channel[1].rxrdy | m_channel[2].rxrdy | m_channel[0].txrdy | m_channel[1].txrdy | m_reset) == ASSERT_LINE)
		return Z80_DAISY_INT;
	else
		return 0;
}

int k1801vp120_sub_device::z80daisy_irq_ack()
{
	int vec = -1;

	if (m_reset == ASSERT_LINE)
	{
		m_reset = CLEAR_LINE;
		vec = 0314;
	}
	else if (m_channel[0].rxrdy == ASSERT_LINE)
	{
		m_channel[0].rxrdy = CLEAR_LINE;
		vec = 0320;
	}
	else if (m_channel[0].txrdy == ASSERT_LINE)
	{
		m_channel[0].txrdy = CLEAR_LINE;
		vec = 0324;
	}
	else if (m_channel[1].rxrdy == ASSERT_LINE)
	{
		m_channel[1].rxrdy = CLEAR_LINE;
		vec = 0330;
	}
	else if (m_channel[1].txrdy == ASSERT_LINE)
	{
		m_channel[1].txrdy = CLEAR_LINE;
		vec = 0334;
	}
	else if (m_channel[2].rxrdy == ASSERT_LINE)
	{
		m_channel[2].rxrdy = CLEAR_LINE;
		vec = 0340;
	}

	return vec;
}


//-------------------------------------------------
//  read - register read
//-------------------------------------------------

uint16_t k1801vp120_sub_device::read(offs_t offset)
{
	uint16_t data = 0;

	switch (offset)
	{
	case 0: case 1: case 2:
		data = m_channel[offset].rbuf;
		if (!machine().side_effects_disabled())
		{
			clear_virq(m_write_virq, m_rcsr, (PPRCSR_0IE << offset), m_channel[offset].rxrdy);
			m_write_ack[offset](ASSERT_LINE);
			m_rcsr &= ~(PPRCSR_0RDY << offset);
		}
		break;

	case 3:
		data = m_rcsr & PPRCSR_RD;
		break;

	case 7:
		data = m_tcsr & PPTCSR_RD;
		break;
	}

	if (0)
	LOG("%s R %06o == %06o\n", machine().describe_context(), 0177060 + (offset << 1), data);

	return data;
}

//-------------------------------------------------
//  write - register write
//-------------------------------------------------

void k1801vp120_sub_device::write(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	LOG("%s W %06o <- %06o & %06o\n", machine().describe_context(), 0177060 + (offset << 1), data, mem_mask);

	switch (offset)
	{
	case 3:
		if ((data & PPRCSR_0IE) == 0)
			clear_virq(m_write_virq, 1, 1, m_channel[0].rxrdy);
		else if ((m_rcsr & (PPRCSR_0RDY + PPRCSR_0IE)) == PPRCSR_0RDY)
			m_channel[0].rxrdy = ASSERT_LINE;

		if ((data & PPRCSR_1IE) == 0)
			clear_virq(m_write_virq, 1, 1, m_channel[1].rxrdy);
		else if ((m_rcsr & (PPRCSR_1RDY + PPRCSR_1IE)) == PPRCSR_1RDY)
			m_channel[1].rxrdy = ASSERT_LINE;

		if ((data & PPRCSR_2IE) == 0)
			clear_virq(m_write_virq, 1, 1, m_channel[2].rxrdy);
		else if ((m_rcsr & (PPRCSR_2RDY + PPRCSR_2IE)) == PPRCSR_2RDY)
			m_channel[2].rxrdy = ASSERT_LINE;

		m_rcsr = ((m_rcsr & ~PPRCSR_WR) | (data & PPRCSR_WR));
		break;

	case 4:
		m_write_out[0](data);
		clear_virq(m_write_virq, m_tcsr, PPTCSR_0IE, m_channel[0].txrdy);
		break;

	case 5:
		m_write_out[1](data);
		clear_virq(m_write_virq, m_tcsr, PPTCSR_1IE, m_channel[1].txrdy);
		break;

	case 7:
		if ((data & PPTCSR_0IE) == 0)
			clear_virq(m_write_virq, 1, 1, m_channel[0].txrdy);
		else if ((m_tcsr & (PPTCSR_0DONE + PPTCSR_0IE)) == PPTCSR_0DONE)
			raise_virq(m_write_virq, 1, 1, m_channel[0].txrdy);

		if ((data & PPTCSR_1IE) == 0)
			clear_virq(m_write_virq, 1, 1, m_channel[1].txrdy);
		else if ((m_tcsr & (PPTCSR_1DONE + PPTCSR_1IE)) == PPTCSR_1DONE)
			raise_virq(m_write_virq, 1, 1, m_channel[1].txrdy);

		m_tcsr = ((m_tcsr & ~PPTCSR_WR) | (data & PPTCSR_WR));
		break;
	}
}
