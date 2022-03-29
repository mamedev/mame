// license:BSD-3-Clause
// copyright-holders:Andrei I. Holub
/**********************************************************************

    TS-Conf (ZX-Evolution) DMA Controller

**********************************************************************/

#include "emu.h"
#include "tsconfdma.h"

tsconfdma_device::tsconfdma_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, TSCONF_DMA, tag, owner, clock),
	  m_in_mreq_cb(*this),
	  m_out_mreq_cb(*this),
	  m_in_mspi_cb(*this),
	  m_out_cram_cb(*this),
	  m_out_sfile_cb(*this),
	  m_on_ready_cb(*this)
{
}

void tsconfdma_device::device_start()
{
	m_in_mreq_cb.resolve_safe(0);
	m_out_mreq_cb.resolve_safe();
	m_in_mspi_cb.resolve_safe(0);
	m_out_cram_cb.resolve_safe();
	m_out_sfile_cb.resolve_safe();
	m_on_ready_cb.resolve_safe();

	save_item(NAME(m_ready));
	save_item(NAME(m_address_s));
	save_item(NAME(m_address_d));
	save_item(NAME(m_block_len));
	save_item(NAME(m_block_num));
	save_item(NAME(m_align_s));
	save_item(NAME(m_align_d));
	save_item(NAME(m_align));
}

void tsconfdma_device::device_reset()
{
	m_block_num = 0;
	m_ready = ASSERT_LINE;
}

int tsconfdma_device::is_ready()
{
	return m_ready;
}

void tsconfdma_device::set_saddr_l(u8 addr_l)
{
	m_address_s = (m_address_s & 0xffffff00) | (addr_l & 0xfe);
}

void tsconfdma_device::set_saddr_h(u8 addr_h)
{
	m_address_s = (m_address_s & 0xffffc0ff) | ((addr_h & 0x3f) << 8);
}

void tsconfdma_device::set_saddr_x(u8 addr_x)
{
	m_address_s = (m_address_s & 0x0003fff) | (addr_x << 14);
}

void tsconfdma_device::set_daddr_l(u8 addr_l)
{
	m_address_d = (m_address_d & 0xffffff00) | (addr_l & 0xfe);
}

void tsconfdma_device::set_daddr_h(u8 addr_h)
{
	m_address_d = (m_address_d & 0xffffc0ff) | ((addr_h & 0x3f) << 8);
}

void tsconfdma_device::set_daddr_x(u8 addr_x)
{
	m_address_d = (m_address_d & 0x0003fff) | (addr_x << 14);
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
	m_ready = CLEAR_LINE;
	m_align = align_opt ? 512 : 256;

	// TODO Transfers 2 byte/cycle at 7MHz
	switch (dev)
	{
	case 0b0001: // Mem -> Mem
		for (u16 block = 0; block <= m_block_num; block++)
		{
			auto s_addr = m_address_s;
			auto d_addr = m_address_d;
			for (u16 len = 0; len <= m_block_len; len++)
			{
				m_out_mreq_cb(d_addr, m_in_mreq_cb(s_addr));
				s_addr += 2;
				d_addr += 2;
			}
			m_address_s = s_align ? (m_address_s + m_align) : s_addr;
			m_address_d = d_align ? (m_address_d + m_align) : d_addr;
		}
		break;

	case 0b0010: // SPI -> Mem
		for (u16 block = 0; block <= m_block_num; block++)
		{
			auto d_addr = m_address_d;
			for (u16 len = 0; len <= m_block_len; len++)
			{
				m_out_mreq_cb(d_addr, m_in_mspi_cb());
				d_addr += 2;
			}
			m_address_d = d_align ? (m_address_d + m_align) : d_addr;
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
				d_addr += 2;
			}
			m_address_d = d_align ? (m_address_d + m_align) : d_addr;
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
				if (d_val != 0)
				{
					m_out_mreq_cb(d_addr, m_in_mreq_cb(s_addr));
				}
				s_addr += 2;
				d_addr += 2;
			}
			m_address_s = s_align ? (m_address_s + m_align) : s_addr;
			m_address_d = d_align ? (m_address_d + m_align) : d_addr;
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
				s_addr += 2;
				d_addr += 2;
			}
			m_address_s = s_align ? (m_address_s + m_align) : s_addr;
			m_address_d = d_align ? (m_address_d + m_align) : d_addr;
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
				s_addr += 2;
				d_addr += 2;
			}
			m_address_s = s_align ? (m_address_s + m_align) : s_addr;
			m_address_d = d_align ? (m_address_d + m_align) : d_addr;
		}
		break;

	default:
		logerror("'tsdma': TX %02X: %06X (%02X:%04X) -> %06X\n", dev, m_address_s, m_block_len, m_block_num, m_address_d);
		break;
	}

	m_ready = ASSERT_LINE;
	m_on_ready_cb(0);
}

// device type definition
DEFINE_DEVICE_TYPE(TSCONF_DMA, tsconfdma_device, "tsconfdma", "TS-Conf DMA Controller")
