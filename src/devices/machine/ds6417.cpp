// license:BSD-3-Clause
// copyright-holders:Carl


#include "emu.h"
#include "ds6417.h"


DEFINE_DEVICE_TYPE(DS6417, ds6417_device, "ds6417", "Dallas DS6417 CyberCard")

ds6417_device::ds6417_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, DS6417, tag, owner, clock)
	, device_image_interface(mconfig, *this)
{
}

void ds6417_device::device_start()
{
	save_item(NAME(m_read));
	save_item(NAME(m_clk));
	save_item(NAME(m_reset));
	save_item(NAME(m_start));
	save_item(NAME(m_data));
	save_item(NAME(m_command));
	save_item(NAME(m_addr));
	save_item(NAME(m_crc));
	save_item(NAME(m_select));
	save_item(NAME(m_shiftreg));
	save_item(NAME(m_count));
}

void ds6417_device::device_reset()
{
	m_read = false;
	m_start = false;
	m_count = 0;
	m_command = 0;
}

image_init_result ds6417_device::call_load()
{
	if(length() != 32768)
		return image_init_result::FAIL;
	return image_init_result::PASS;
}

image_init_result ds6417_device::call_create(int format_type, util::option_resolution *format_options)
{
	u8 buffer[32768] = {0};
	if(fwrite(buffer, 32768) != 32768)
		return image_init_result::FAIL;
	return image_init_result::PASS;
}

uint8_t ds6417_device::calccrc(uint8_t bit, uint8_t crc) const
{
	bit = (crc ^ bit) & 1;
	if(bit)
		return ((crc >> 1) | (bit << 7)) ^ 0x66;
	else
		return crc >> 1;
}

WRITE_LINE_MEMBER( ds6417_device::clock_w )
{
	if(!m_reset || !exists())
		return;

	if(m_clk == (state != 0))
		return;

	m_clk = state;

	if(m_read && !m_clk && m_start)
	{
		if(!(m_count & 7))
		{
			switch(m_command)
			{
				case CMD_READ:
				case CMD_READMASK:
					fread(&m_shiftreg, 1);
					break;
				case CMD_READPROT:
					m_shiftreg = m_selectval;
					break;
				case CMD_READCRC:
					m_shiftreg = m_crc;
					break;
			}
		}

		m_data = m_shiftreg & 1;
		m_shiftreg >>= 1;
		m_count++;
		m_crc = calccrc(m_data, m_crc);
	}
	else if(m_clk && !m_read)
	{
		m_shiftreg = (m_shiftreg >> 1) | (m_data ? 0x80 : 0);
		m_count++;

		if(m_start)
		{
			m_crc = calccrc(m_data, m_crc);
			if(!(m_count & 7))
			{
				switch(m_command)
				{
					case CMD_WRITE:
						fwrite(&m_shiftreg, 1);
						break;
					case CMD_WRITEPROT:
						m_selectval = m_shiftreg;
						break;
				}
			}
		}
		else
		{
			switch(m_count)
			{
				case 8:
					if((m_shiftreg != 0xe8) && (m_shiftreg != 0x17))
						reset();
					break;
				case 16:
					m_addr = m_shiftreg;
					break;
				case 24:
					m_addr |= m_shiftreg << 8;
					break;
				case 32:
					m_addr |= (m_shiftreg & 7) << 16;
					m_command = m_shiftreg >> 3;
					break;
				case 40:
					m_select = m_shiftreg;
					break;
				case 48:
					m_select |= m_shiftreg << 8;
					break;
				case 56:
					// command crc
					if((m_command & CMD_READMASK) == CMD_READMASK)
					{
						m_selbits = m_command & 7;
						m_command &= 0x18;
					}
					switch(m_command)
					{
						case CMD_READ:
						case CMD_READMASK:
							m_crc = 0; [[fallthrough]];
						case CMD_READPROT:
						case CMD_READCRC:
							m_read = true;
							break;
						case CMD_WRITE:
							m_crc = 0; [[fallthrough]];
						case CMD_WRITEPROT:
							break;
						default:
							reset();
							return;

					}
					m_start = true;
					fseek(m_addr & 0x7fff, SEEK_SET);
					break;
			}
		}
	}
}
