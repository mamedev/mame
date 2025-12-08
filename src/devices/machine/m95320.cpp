// license: BSD-3-Clause
// copyright-holders: Fabio Priuli, MetalliC
/**************************************************************************************************

M95320-W M95320-R M95320-DF
32-Kbit serial SPI bus EEPROM with high-speed clock

TODO:
- direct converted from original stm95 for megadriv:psolar;
- Actual SPI bus connection;
- M95320-D variant (has extra instructions);
- M95640 (same base instruction set, double size);

**************************************************************************************************/

#include "emu.h"
#include "m95320.h"

DEFINE_DEVICE_TYPE(M95320_EEPROM, m95320_eeprom_device, "m95320_eeprom", "STM95320 SPI bus EEPROM")

m95320_eeprom_device::m95320_eeprom_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_nvram_interface(mconfig, *this)
{
}

m95320_eeprom_device::m95320_eeprom_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: m95320_eeprom_device(mconfig, M95320_EEPROM, tag, owner, clock)
{
}


void m95320_eeprom_device::device_start()
{
	m_eeprom_data = std::make_unique<uint8_t[]>(m_size);

	save_pointer(NAME(m_eeprom_data), m_size);
	save_item(NAME(m_latch));
	save_item(NAME(m_reset_line));
	save_item(NAME(m_sck_line));
	save_item(NAME(m_wel));
	save_item(NAME(m_stream_pos));
	save_item(NAME(m_stream_data));
	save_item(NAME(m_eeprom_addr));
}

void m95320_eeprom_device::nvram_default()
{
}


bool m95320_eeprom_device::nvram_read(util::read_stream &file)
{
	auto const [err, actual] = util::read(file, m_eeprom_data.get(), m_size);
	return !err && (actual == m_size);
}

bool m95320_eeprom_device::nvram_write(util::write_stream &file)
{
	auto const [err, actual] = util::write(file, m_eeprom_data.get(), m_size);
	return !err;
}


void m95320_eeprom_device::set_cs_line(int state)
{
	m_reset_line = state;
	if (m_reset_line != CLEAR_LINE)
	{
		m_stream_pos = 0;
		m_internal_state = IDLE;
	}
}

void m95320_eeprom_device::set_si_line(int state)
{
	m_latch = state;
}

int m95320_eeprom_device::get_so_line(void)
{
	if (m_internal_state == READING || m_internal_state == CMD_RDSR)
		return (m_stream_data >> 8) & 1;
	else
		return 0;
}

void m95320_eeprom_device::set_sck_line(int state)
{
	if (m_reset_line == CLEAR_LINE)
	{
		if (state == ASSERT_LINE && m_sck_line == CLEAR_LINE)
		{
			switch (m_internal_state)
			{
				case IDLE:
					m_stream_data = (m_stream_data << 1) | (m_latch ? 1 : 0);
					m_stream_pos++;
					if (m_stream_pos == 8)
					{
						m_stream_pos = 0;
						//printf("STM95 EEPROM: got cmd %02X\n", m_stream_data&0xff);
						switch(m_stream_data & 0xff)
						{
							case 0x01:  // write status register
								if (m_wel != 0)
									m_internal_state = CMD_WRSR;
								m_wel = 0;
								break;
							case 0x02:  // write
								if (m_wel != 0)
									m_internal_state = CMD_WRITE;
								m_stream_data = 0;
								m_wel = 0;
								break;
							case 0x03:  // read
								m_internal_state = CMD_READ;
								m_stream_data = 0;
								break;
							case 0x04:  // write disable
								m_wel = 0;
								break;
							case 0x05:  // read status register
								m_internal_state = CMD_RDSR;
								// TODO: SRWD / BP1 / BP0 and WIP bits
								m_stream_data = m_wel << 1;
								break;
							case 0x06:  // write enable
								m_wel = 1;
								break;
							default:
								logerror("unknown cmd %02X\n", m_stream_data&0xff);
						}
					}
					break;
				case CMD_WRSR:
					m_stream_pos++;       // just skip, don't care block protection
					if (m_stream_pos == 8)
					{
						m_internal_state = IDLE;
						m_stream_pos = 0;
					}
					break;
				case CMD_RDSR:
					m_stream_data = m_stream_data << 1;
					m_stream_pos++;
					if (m_stream_pos == 8)
					{
						m_internal_state = IDLE;
						m_stream_pos = 0;
					}
					break;
				case CMD_READ:
					m_stream_data = (m_stream_data << 1) | (m_latch ? 1 : 0);
					m_stream_pos++;
					if (m_stream_pos == 16)
					{
						m_eeprom_addr = m_stream_data & (m_size - 1);
						m_stream_data = m_eeprom_data[m_eeprom_addr];
						m_internal_state = READING;
						m_stream_pos = 0;
					}
					break;
				case READING:
					m_stream_data = m_stream_data<<1;
					m_stream_pos++;
					if (m_stream_pos == 8)
					{
						if (++m_eeprom_addr == m_size)
							m_eeprom_addr = 0;
						m_stream_data |= m_eeprom_data[m_eeprom_addr];
						m_stream_pos = 0;
					}
					break;
				case CMD_WRITE:
					m_stream_data = (m_stream_data << 1) | (m_latch ? 1 : 0);
					m_stream_pos++;
					if (m_stream_pos == 16)
					{
						m_eeprom_addr = m_stream_data & (m_size - 1);
						m_internal_state = WRITING;
						m_stream_pos = 0;
					}
					break;
				case WRITING:
					m_stream_data = (m_stream_data << 1) | (m_latch ? 1 : 0);
					m_stream_pos++;
					if (m_stream_pos == 8)
					{
						m_eeprom_data[m_eeprom_addr] = m_stream_data;
						if (++m_eeprom_addr == m_size)
							m_eeprom_addr = 0;
						m_stream_pos = 0;
					}
					break;
			}
		}
	}
	m_sck_line = state;
}


