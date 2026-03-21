// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
/**********************************************************************

    Hudson/NEC HuC6202 Video Priority Controller

    TODO:
    - Verify window area calculation
    - Verify overscan color

**********************************************************************/

#include "emu.h"
#include "huc6202.h"

#include "huc6270.h"


DEFINE_DEVICE_TYPE(HUC6202, huc6202_device, "huc6202", "Hudson HuC6202 VPC")


huc6202_device::huc6202_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, HUC6202, tag, owner, clock)
	, m_next_pixel_cb(*this, 0)
	, m_time_til_next_event_cb(*this, 1)
	, m_read_cb(*this, 0)
	, m_write_cb(*this)
	, m_window{0}
	, m_io_device(false)
	, m_map_index(0)
	, m_map_dirty(false)
	, m_prio_map{0}
{
}


u16 huc6202_device::next_pixel()
{
	u16 data = huc6270_device::HUC6270_BACKGROUND;

	u16 const data_0 = m_next_pixel_cb[0]();
	u16 const data_1 = m_next_pixel_cb[1]();

	if (data_0 == huc6270_device::HUC6270_SPRITE && data_1 == huc6270_device::HUC6270_SPRITE)
	{
		m_map_index = 0;
		if (m_map_dirty)
		{
			for (int i = 0; i < 512; i++)
			{
				m_prio_map[i] = 0;
				if (m_window[0] < 0x40 || i > m_window[0])
				{
					m_prio_map[i] |= 1;
				}
				if (m_window[1] < 0x40 || i > m_window[1])
				{
					m_prio_map[i] |= 2;
				}
			}
			m_map_dirty = false;
		}
	}
	else
	{
		u8 const prio_index = m_prio_map[m_map_index];
		bool const dev0_en = m_prio[prio_index].dev0_enabled && data_0 != huc6270_device::HUC6270_SPRITE;
		bool const dev1_en = m_prio[prio_index].dev1_enabled && data_1 != huc6270_device::HUC6270_SPRITE;

		if (dev0_en)
		{
			if (dev1_en)
			{
				switch (m_prio[prio_index].prio_type)
				{
				default:
				case 0:     /* Back - BG1 SP1 BG0 SP0 - Front */
					data = (data_0 & 0x0f) ? data_0 : data_1;
					break;

				case 1:     /* Back - BG1 BG0 SP1 SP0 - Front */
					if (data_0 > huc6270_device::HUC6270_SPRITE)
					{
						/* Device 0 sprite */
						data = data_0;
					}
					else if (data_1 > huc6270_device::HUC6270_SPRITE)
					{
						/* Device 1 sprite */
						data = data_1;
					}
					else
					{
						/* Device 0 and 1 backgrounds */
						data = (data_0 & 0x0f) ? data_0 : data_1;
					}
					break;

				case 2:     /* Back - BG0 + SP1 => BG0 - Front
				                      BG0 + BG1 => BG0
				                      BG1 + SP0 => BG1
				                      SP0 + SP1 => SP0
				            */
					if (data_1 > huc6270_device::HUC6270_SPRITE)
					{
						if (data_0 > huc6270_device::HUC6270_SPRITE)
						{
							/* Device 1 sprite, device 0 sprite */
							data = data_0;
						}
						else
						{
							/* Device 1 sprite, device 0 background */
							data = (data_0 & 0x0f) ? data_0 : data_1;
						}
					}
					else
					{
						if (data_0 > huc6270_device::HUC6270_SPRITE)
						{
							/* Device 1 background, device 0 sprite */
							data = data_1;
						}
						else
						{
							/* Device 1 background, device 0 background */
							data = (data_0 & 0x0f) ? data_0 : data_1;
						}
					}
					break;
				}
			}
			else
			{
				/* Only device 0 is enabled */
				data = data_0;
			}
		}
		else
		{
			/* Only device 1 is enabled */
			if (dev1_en)
			{
				data = data_1;
			}
		}
		m_map_index += 1;
	}
	return data;
}


u16 huc6202_device::time_until_next_event()
{
	u16 const next_event_clocks_0 = m_time_til_next_event_cb[0]();
	u16 const next_event_clocks_1 = m_time_til_next_event_cb[1]();

	return std::min(next_event_clocks_0, next_event_clocks_1);
}


u8 huc6202_device::read(offs_t offset)
{
	u8 data = 0xff;

	switch (offset & 7)
	{
		case 0x00:  /* Priority register #0 */
			data = (m_prio[0].prio_type << 2) |
				(m_prio[0].dev0_enabled ? 0x01 : 0) |
				(m_prio[0].dev1_enabled ? 0x02 : 0) |
				(m_prio[1].prio_type << 6) |
				(m_prio[1].dev0_enabled ? 0x10 : 0) |
				(m_prio[1].dev1_enabled ? 0x20 : 0);
			break;

		case 0x01:  /* Priority register #1 */
			data = (m_prio[2].prio_type << 2) |
				(m_prio[2].dev0_enabled ? 0x01 : 0) |
				(m_prio[2].dev1_enabled ? 0x02 : 0) |
				(m_prio[3].prio_type << 6) |
				(m_prio[3].dev0_enabled ? 0x10 : 0) |
				(m_prio[3].dev1_enabled ? 0x20 : 0);
			break;

		case 0x02:  /* Window 1 LSB */
			data = m_window[0] & 0xff;
			break;

		case 0x03:  /* Window 1 MSB */
			data = (m_window[0] >> 8) & 0xff;
			break;

		case 0x04:  /* Window 2 LSB */
			data = m_window[1] & 0xff;
			break;

		case 0x05:  /* Window 2 MSB */
			data = (m_window[1] >> 8) & 0xff;
			break;
	}

	return data;
}


void huc6202_device::write(offs_t offset, u8 data)
{
	switch (offset & 7)
	{
		case 0x00:  /* Priority register #0 */
			m_prio[0].dev0_enabled = BIT(data, 0);
			m_prio[0].dev1_enabled = BIT(data, 1);
			m_prio[0].prio_type = (data >> 2) & 0x03;
			m_prio[1].dev0_enabled = BIT(data, 4);
			m_prio[1].dev1_enabled = BIT(data, 5);
			m_prio[1].prio_type = (data >> 6) & 0x03;
			break;

		case 0x01:  /* Priority register #1 */
			m_prio[2].dev0_enabled = BIT(data, 0);
			m_prio[2].dev1_enabled = BIT(data, 1);
			m_prio[2].prio_type = (data >> 2) & 0x03;
			m_prio[3].dev0_enabled = BIT(data, 4);
			m_prio[3].dev1_enabled = BIT(data, 5);
			m_prio[3].prio_type = (data >> 6) & 0x03;
			break;

		case 0x02:  /* Window 1 LSB */
			m_window[0] = (m_window[0] & 0xff00) | data;
			m_map_dirty = true;
			break;

		case 0x03:  /* Window 1 MSB */
			m_window[0] = ((m_window[0] & 0x00ff) | (data << 8)) & 0x3ff;
			m_map_dirty = true;
			break;

		case 0x04:  /* Window 2 LSB */
			m_window[1] = (m_window[1] & 0xff00) | data;
			m_map_dirty = true;
			break;

		case 0x05:  /* Window 2 MSB */
			m_window[1] = ((m_window[1] & 0x00ff) | (data << 8)) & 0x3ff;
			m_map_dirty = true;
			break;

		case 0x06:  /* I/O select */
			m_io_device = BIT(data, 0);
			break;
	}
}


u8 huc6202_device::io_read(offs_t offset)
{
	return m_read_cb[m_io_device ? 1 : 0](offset);
}


void huc6202_device::io_write(offs_t offset, u8 data)
{
	m_write_cb[m_io_device ? 1 : 0](offset, data);
}


void huc6202_device::device_start()
{
	/* We want all our callbacks to be resolved */
	assert(!m_next_pixel_cb[0].isunset());
	assert(!m_time_til_next_event_cb[0].isunset());
	assert(!m_read_cb[0].isunset());
	assert(!m_write_cb[0].isunset());
	assert(!m_next_pixel_cb[1].isunset());
	assert(!m_time_til_next_event_cb[1].isunset());
	assert(!m_read_cb[1].isunset());
	assert(!m_write_cb[1].isunset());

	/* Register save items */
	save_item(STRUCT_MEMBER(m_prio, prio_type));
	save_item(STRUCT_MEMBER(m_prio, dev0_enabled));
	save_item(STRUCT_MEMBER(m_prio, dev1_enabled));
	save_item(NAME(m_window));
	save_item(NAME(m_io_device));
	save_item(NAME(m_map_index));
	save_item(NAME(m_map_dirty));
	save_item(NAME(m_prio_map));
}


void huc6202_device::device_reset()
{
	m_prio[0].prio_type = 0;
	m_prio[0].dev0_enabled = true;
	m_prio[0].dev1_enabled = false;
	m_prio[1].prio_type = 0;
	m_prio[1].dev0_enabled = true;
	m_prio[1].dev1_enabled = false;
	m_prio[2].prio_type = 0;
	m_prio[2].dev0_enabled = true;
	m_prio[2].dev1_enabled = false;
	m_prio[3].prio_type = 0;
	m_prio[3].dev0_enabled = true;
	m_prio[3].dev1_enabled = false;
	m_map_dirty = true;
	m_window[0] = 0;
	m_window[1] = 0;
	m_io_device = false;
}
