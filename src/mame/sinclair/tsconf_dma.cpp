// license:BSD-3-Clause
// copyright-holders:Andrei I. Holub
/**********************************************************************

    TS-Conf (ZX-Evolution) DMA Controller

TODO:
* Each memory cycle aligned to 7MHz clock and taking 1 tick

**********************************************************************/

#include "emu.h"
#include "tsconf_dma.h"

//#define VERBOSE ( LOG_GENERAL )
#include "logmacro.h"

enum
{
	SEQ_READ,
	SEQ_WRITE
};

enum
{
	MEM_MEM   = 0b0001,
	SPI_MEM   = 0b0010,
	FILL_MEM  = 0b0100,
	BLT_MEM   = 0b1001,
	RAM_CRAM  = 0b1100,
	RAM_SFILE = 0b1101
};

tsconfdma_device::tsconfdma_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, TSCONF_DMA, tag, owner, clock)
	, m_in_mreq_cb(*this, 0)
	, m_out_mreq_cb(*this)
	, m_in_mspi_cb(*this, 0)
	, m_out_cram_cb(*this)
	, m_out_sfile_cb(*this)
	, m_on_ready_cb(*this)
{
}

void tsconfdma_device::device_start()
{
	m_dma_clock = timer_alloc(FUNC(tsconfdma_device::dma_clock), this);

	save_item(NAME(m_ready));
	save_item(NAME(m_address_s));
	save_item(NAME(m_address_d));
	save_item(NAME(m_block_len));
	save_item(NAME(m_block_num));
	save_item(NAME(m_tx_s_addr));
	save_item(NAME(m_tx_d_addr));
	save_item(NAME(m_tx_data));
	save_item(NAME(m_tx_block_num));
	save_item(NAME(m_tx_block));
	save_item(NAME(m_task));
	save_item(NAME(m_align_s));
	save_item(NAME(m_align_d));
	save_item(NAME(m_asz));
}

void tsconfdma_device::device_reset()
{
	m_dma_clock->reset();

	m_block_num = 0;
	m_ready = ASSERT_LINE;
}

int tsconfdma_device::is_ready()
{
	return m_ready;
}

void tsconfdma_device::set_saddr_l(u8 addr_l)
{
	m_address_s = (m_address_s & 0x3fff00) | (addr_l & 0xfe);
}

void tsconfdma_device::set_saddr_h(u8 addr_h)
{
	m_address_s = (m_address_s & 0x3fc0ff) | ((addr_h & 0x3f) << 8);
}

void tsconfdma_device::set_saddr_x(u8 addr_x)
{
	m_address_s = (m_address_s & 0x003fff) | (addr_x << 14);
}

void tsconfdma_device::set_daddr_l(u8 addr_l)
{
	m_address_d = (m_address_d & 0x3fff00) | (addr_l & 0xfe);
}

void tsconfdma_device::set_daddr_h(u8 addr_h)
{
	m_address_d = (m_address_d & 0x3fc0ff) | ((addr_h & 0x3f) << 8);
}

void tsconfdma_device::set_daddr_x(u8 addr_x)
{
	m_address_d = (m_address_d & 0x003fff) | (addr_x << 14);
}

void tsconfdma_device::set_block_len(u8 len)
{
	// 1..256
	m_block_len = len;
}

void tsconfdma_device::set_block_num_l(u8 num_l)
{
	m_block_num = (m_block_num & 0xff00) | num_l;
}

void tsconfdma_device::set_block_num_h(u8 num_h)
{
	// 1..1025 * 2
	m_block_num = (m_block_num & 0x00ff) | ((num_h & 0x03) << 8);
}

void tsconfdma_device::start_tx(u8 task, bool s_align, bool d_align, bool align_opt)
{
	if (!m_ready)
		LOG("Starting new tx without previous completed\n");

	m_task = task;
	m_align_s = s_align;
	m_align_d = d_align;
	m_asz = align_opt;
	m_ready = CLEAR_LINE;

	m_tx_block_num = 0;
	m_tx_block = 0;

	const attotime curtime = machine().time();
	m_dma_clock->adjust(attotime::from_ticks(curtime.as_ticks(clock()) + 1, clock()) - curtime, SEQ_READ);
}

TIMER_CALLBACK_MEMBER(tsconfdma_device::dma_clock)
{
	int len;
	if (param == SEQ_READ)
	{
		if (m_tx_block == 0)
		{
			m_tx_s_addr = m_address_s;
			m_tx_d_addr = m_address_d;
		}

		len = do_read();
	}
	else
	{
		len = do_write();

		const u32 m_m1 = m_asz ? 0x3ffe00 : 0x3fff00;
		const u32 m_m2 = m_asz ? 0x0001ff : 0x0000ff;
		const u16 m_align = m_asz ? 512 : 256;
		m_tx_s_addr = m_align_s ? ((m_tx_s_addr & m_m1) | ((m_tx_s_addr + 2) & m_m2)) : ((m_tx_s_addr + 2) & 0x3fffff);
		m_tx_d_addr = m_align_d ? ((m_tx_d_addr & m_m1) | ((m_tx_d_addr + 2) & m_m2)) : ((m_tx_d_addr + 2) & 0x3fffff);

		++m_tx_block;
		if (m_tx_block > m_block_len)
		{
			m_address_s = m_align_s ? (m_address_s + m_align) : m_tx_s_addr;
			m_address_d = m_align_d ? (m_address_d + m_align) : m_tx_d_addr;

			m_tx_block = 0;
			++m_tx_block_num;
			if (m_tx_block_num > m_block_num)
			{
				m_dma_clock->reset();
				m_ready = ASSERT_LINE;
				m_on_ready_cb(1);
				return;
			}
		}
	}

	if (len >= 0)
		m_dma_clock->adjust(clocks_to_attotime(len), param == SEQ_READ ? SEQ_WRITE : SEQ_READ);
	else
		LOG("Unknown task %02X: %06X (%02X:%04X) -> %06X\n", m_task, m_address_s, m_block_len, m_block_num, m_address_d);
}

int tsconfdma_device::do_read()
{
	int len = -1;
	switch (m_task)
	{
	case MEM_MEM:
	case RAM_CRAM:
	case RAM_SFILE:
		len = 4;
		m_tx_data = m_in_mreq_cb(m_tx_s_addr);
		break;

	case SPI_MEM:
		len = 4;
		m_tx_data = m_in_mspi_cb();
		break;

	case FILL_MEM:
		if (!m_tx_block_num && !m_tx_block)
		{
			len = 4;
			m_tx_data = m_in_mreq_cb(m_address_s);
		}
		else
			len = 0;
		break;

	case BLT_MEM:
		{
			len = 8;
			u16 d_val = m_in_mreq_cb(m_tx_d_addr);
			u16 s_val = m_in_mreq_cb(m_tx_s_addr);
			if (m_asz)
			{
				d_val = (d_val & 0xff00) | (((s_val & 0x00ff) ? s_val : d_val) & 0x00ff);
				d_val = (d_val & 0x00ff) | (((s_val & 0xff00) ? s_val : d_val) & 0xff00);
			}
			else
			{
				d_val = (d_val & 0xfff0) | (((s_val & 0x000f) ? s_val : d_val) & 0x000f);
				d_val = (d_val & 0xff0f) | (((s_val & 0x00f0) ? s_val : d_val) & 0x00f0);
				d_val = (d_val & 0xf0ff) | (((s_val & 0x0f00) ? s_val : d_val) & 0x0f00);
				d_val = (d_val & 0x0fff) | (((s_val & 0xf000) ? s_val : d_val) & 0xf000);
			}
			m_tx_data = d_val;
		}
		break;

	default:
		break;
	}

	return len;
}

int tsconfdma_device::do_write()
{
	int len = -1;
	switch (m_task)
	{
	case MEM_MEM:
	case SPI_MEM:
	case FILL_MEM:
	case BLT_MEM:
		len = 4;
		m_out_mreq_cb(m_tx_d_addr, m_tx_data);
		break;

	case RAM_CRAM:
		len = 1;
		m_out_cram_cb(m_tx_d_addr, m_tx_data);
		break;

	case RAM_SFILE:
		len = 4;
		m_out_sfile_cb(m_tx_d_addr, m_tx_data);
		break;

	default:
		break;
	}

	return len;
}

// device type definition
DEFINE_DEVICE_TYPE(TSCONF_DMA, tsconfdma_device, "tsconfdma", "TS-Conf DMA Controller")
