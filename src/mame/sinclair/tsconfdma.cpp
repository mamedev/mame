// license:BSD-3-Clause
// copyright-holders:Andrei I. Holub
/**********************************************************************

    TS-Conf (ZX-Evolution) DMA Controller

**********************************************************************/

#include "emu.h"
#include "tsconfdma.h"

tsconfdma_device::tsconfdma_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, TSCONF_DMA, tag, owner, clock),
	  m_in_mreq_cb(*this, 0),
	  m_out_mreq_cb(*this),
	  m_in_mspi_cb(*this, 0),
	  m_out_cram_cb(*this),
	  m_out_sfile_cb(*this),
	  m_on_ready_cb(*this)
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
	save_item(NAME(m_align_s));
	save_item(NAME(m_align_d));
	save_item(NAME(m_align));
	save_item(NAME(m_m1));
	save_item(NAME(m_m2));
	save_item(NAME(m_asz));
	save_item(NAME(m_task));
}

void tsconfdma_device::device_reset()
{
	m_dma_clock->adjust(attotime::never);

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

void tsconfdma_device::start_tx(u8 dev, bool s_align, bool d_align, bool align_opt)
{
	m_task = dev;
	m_align_s = s_align;
	m_align_d = d_align;
	m_asz = align_opt;
	m_align = m_asz ? 512 : 256;
	m_m1 = m_asz ? 0x3ffe00 : 0x3fff00;
	m_m2 = m_asz ? 0x0001ff : 0x0000ff;
	m_ready = CLEAR_LINE;

	// TODO Transfers 2 byte/cycle at 7MHz
	m_dma_clock->adjust(attotime::from_ticks(m_block_num + 1, 7_MHz_XTAL));
}

TIMER_CALLBACK_MEMBER(tsconfdma_device::dma_clock)
{
	switch (m_task)
	{
	case 0b0001: // Mem -> Mem
		for (u16 block = 0; block <= m_block_num; block++)
		{
			auto s_addr = m_address_s;
			auto d_addr = m_address_d;
			for (u16 len = 0; len <= m_block_len; len++)
			{
				m_out_mreq_cb(d_addr, m_in_mreq_cb(s_addr));
				s_addr = m_align_s ? ((s_addr & m_m1) | ((s_addr + 2) & m_m2)) : ((s_addr + 2) & 0x3fffff);
				d_addr = m_align_d ? ((d_addr & m_m1) | ((d_addr + 2) & m_m2)) : ((d_addr + 2) & 0x3fffff);
			}
			m_address_s = m_align_s ? (m_address_s + m_align) : s_addr;
			m_address_d = m_align_d ? (m_address_d + m_align) : d_addr;
		}
		break;

	case 0b0010: // SPI -> Mem
		for (u16 block = 0; block <= m_block_num; block++)
		{
			auto d_addr = m_address_d;
			for (u16 len = 0; len <= m_block_len; len++)
			{
				m_out_mreq_cb(d_addr, m_in_mspi_cb());
				d_addr = m_align_d ? ((d_addr & m_m1) | ((d_addr + 2) & m_m2)) : ((d_addr + 2) & 0x3fffff);
			}
			m_address_d = m_align_d ? (m_address_d + m_align) : d_addr;
		}
		break;

	case 0b0100: // Fill
		for (u16 block = 0; block <= m_block_num; block++)
		{
			u16 data = m_in_mreq_cb(m_address_s);
			auto d_addr = m_address_d;
			for (u16 len = 0; len <= m_block_len; len++)
			{
				m_out_mreq_cb(d_addr, data);
				d_addr = m_align_d ? ((d_addr & m_m1) | ((d_addr + 2) & m_m2)) : ((d_addr + 2) & 0x3fffff);
			}
			m_address_d = m_align_d ? (m_address_d + m_align) : d_addr;
		}
		break;

	case 0b1001: // Blt -> Mem
		for (u16 block = 0; block <= m_block_num; block++)
		{
			auto s_addr = m_address_s;
			auto d_addr = m_address_d;
			for (u16 len = 0; len <= m_block_len; len++)
			{
				u16 d_val = m_in_mreq_cb(d_addr);
				u16 s_val = m_in_mreq_cb(s_addr);
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
				m_out_mreq_cb(d_addr, d_val);
				s_addr = m_align_s ? ((s_addr & m_m1) | ((s_addr + 2) & m_m2)) : ((s_addr + 2) & 0x3fffff);
				d_addr = m_align_d ? ((d_addr & m_m1) | ((d_addr + 2) & m_m2)) : ((d_addr + 2) & 0x3fffff);
			}
			m_address_s = m_align_s ? (m_address_s + m_align) : s_addr;
			m_address_d = m_align_d ? (m_address_d + m_align) : d_addr;
		}
		break;

	case 0b1100: // RAM -> CRAM
		for (u16 block = 0; block <= m_block_num; block++)
		{
			auto s_addr = m_address_s;
			auto d_addr = m_address_d;
			for (u16 len = 0; len <= m_block_len; len++)
			{
				m_out_cram_cb(d_addr, m_in_mreq_cb(s_addr));
				s_addr = m_align_s ? ((s_addr & m_m1) | ((s_addr + 2) & m_m2)) : ((s_addr + 2) & 0x3fffff);
				d_addr = m_align_d ? ((d_addr & m_m1) | ((d_addr + 2) & m_m2)) : ((d_addr + 2) & 0x3fffff);
			}
			m_address_s = m_align_s ? (m_address_s + m_align) : s_addr;
			m_address_d = m_align_d ? (m_address_d + m_align) : d_addr;
		}
		break;

	case 0b1101: // RAM -> SFILE
		for (u16 block = 0; block <= m_block_num; block++)
		{
			auto s_addr = m_address_s;
			auto d_addr = m_address_d;
			for (u16 len = 0; len <= m_block_len; len++)
			{
				m_out_sfile_cb(d_addr, m_in_mreq_cb(s_addr));
				s_addr = m_align_s ? ((s_addr & m_m1) | ((s_addr + 2) & m_m2)) : ((s_addr + 2) & 0x3fffff);
				d_addr = m_align_d ? ((d_addr & m_m1) | ((d_addr + 2) & m_m2)) : ((d_addr + 2) & 0x3fffff);
			}
			m_address_s = m_align_s ? (m_address_s + m_align) : s_addr;
			m_address_d = m_align_d ? (m_address_d + m_align) : d_addr;
		}
		break;

	default:
		logerror("'tsdma': TX %02X: %06X (%02X:%04X) -> %06X\n", m_task, m_address_s, m_block_len, m_block_num, m_address_d);
		break;
	}

	m_dma_clock->adjust(attotime::never);
	m_ready = ASSERT_LINE;
	m_on_ready_cb(0);
}

// device type definition
DEFINE_DEVICE_TYPE(TSCONF_DMA, tsconfdma_device, "tsconfdma", "TS-Conf DMA Controller")
