// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
/***********************************************************************************************************


 Bandai Wonderswan / Wonderswan Color cart emulation


 ***********************************************************************************************************/


#include "emu.h"
#include "rom.h"

enum
{
	EEPROM_1K, EEPROM_8K, EEPROM_16K
};

//-------------------------------------------------
//  ws_rom_device - constructor
//-------------------------------------------------

DEFINE_DEVICE_TYPE(WS_ROM_STD,    ws_rom_device,        "ws_rom",    "Wonderswan Standard Carts")
DEFINE_DEVICE_TYPE(WS_ROM_SRAM,   ws_rom_sram_device,   "ws_sram",   "Wonderswan Carts w/SRAM")
DEFINE_DEVICE_TYPE(WS_ROM_EEPROM, ws_rom_eeprom_device, "ws_eeprom", "Wonderswan Carts w/EEPROM")
DEFINE_DEVICE_TYPE(WS_ROM_WWITCH, ws_wwitch_device,     "ws_wwitch", "WonderWitch")


ws_rom_device::ws_rom_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock) :
	device_t(mconfig, type, tag, owner, clock),
	device_ws_cart_interface(mconfig, *this),
	m_base20(0),
	m_base30(0),
	m_base40(0),
	m_rtc_setting(0),
	m_rtc_year(0),
	m_rtc_month(0),
	m_rtc_day(0),
	m_rtc_day_of_week(0),
	m_rtc_hour(0),
	m_rtc_minute(0),
	m_rtc_second(0),
	m_rtc_index(0),
	rtc_timer(nullptr)
{
}

ws_rom_device::ws_rom_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	ws_rom_device(mconfig, WS_ROM_STD, tag, owner, clock)
{
}

ws_rom_sram_device::ws_rom_sram_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	ws_rom_device(mconfig, WS_ROM_SRAM, tag, owner, clock),
	m_nvram_base(0)
{
}

ws_rom_sram_device::ws_rom_sram_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock) :
	ws_rom_device(mconfig, type, tag, owner, clock),
	m_nvram_base(0)
{
}

ws_rom_eeprom_device::ws_rom_eeprom_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	ws_rom_device(mconfig, WS_ROM_EEPROM, tag, owner, clock),
	m_eeprom_mode(0), m_eeprom_address(0), m_eeprom_command(0), m_eeprom_start(0), m_eeprom_write_enabled(0)
{
}


ws_wwitch_device::ws_wwitch_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	ws_rom_sram_device(mconfig, WS_ROM_WWITCH, tag, owner, clock)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void ws_rom_device::device_start()
{
	save_item(NAME(m_base20));
	save_item(NAME(m_base30));
	save_item(NAME(m_base40));
	save_item(NAME(m_io_regs));

	// Set up RTC timer
	if (m_has_rtc)
	{
		rtc_timer = timer_alloc(TIMER_RTC);
		rtc_timer->adjust(attotime::zero, 0, attotime::from_seconds(1));
	}

	save_item(NAME(m_rtc_setting));
	save_item(NAME(m_rtc_year));
	save_item(NAME(m_rtc_month));
	save_item(NAME(m_rtc_day));
	save_item(NAME(m_rtc_day_of_week));
	save_item(NAME(m_rtc_hour));
	save_item(NAME(m_rtc_minute));
	save_item(NAME(m_rtc_second));
	save_item(NAME(m_rtc_index));
	save_item(NAME(m_rom_mask));
}

void ws_rom_device::device_reset()
{
	m_rom_mask = (m_rom_size >> 1) - 1;
	m_base20 = ((0xff & m_bank_mask) << 15) & m_rom_mask;
	m_base30 = ((0xff & m_bank_mask) << 15) & m_rom_mask;
	m_base40 = (((0xf0 & m_bank_mask) | 4) << 15) & m_rom_mask;

	memset(m_io_regs, 0xff, sizeof(m_io_regs));

	// Initialize RTC
	m_rtc_index = 0;
	m_rtc_year = 0;
	m_rtc_month = 0;
	m_rtc_day = 0;
	m_rtc_day_of_week = 0;
	m_rtc_hour = 0;
	m_rtc_minute = 0;
	m_rtc_second = 0;
	m_rtc_setting = 0xff;
}

void ws_rom_sram_device::device_start()
{
	save_item(NAME(m_nvram_base));
	ws_rom_device::device_start();
}

void ws_rom_sram_device::device_reset()
{
	m_nvram_base = 0;
	ws_rom_device::device_reset();
}

void ws_rom_eeprom_device::device_start()
{
	ws_rom_device::device_start();

	save_item(NAME(m_eeprom_address));
	save_item(NAME(m_eeprom_command));
	save_item(NAME(m_eeprom_start));
	save_item(NAME(m_eeprom_write_enabled));
}

void ws_wwitch_device::device_start()
{
	save_item(NAME(m_flash_seq));
	save_item(NAME(m_flash_command));
	save_item(NAME(m_write_flash));
	save_item(NAME(m_writing_flash));
	save_item(NAME(m_write_resetting));
	save_item(NAME(m_flash_mode));
	save_item(NAME(m_flash_status));
	ws_rom_sram_device::device_start();
}

void ws_wwitch_device::device_reset()
{
	m_flash_seq = 0;
	m_flash_command = 0;
	m_write_flash = false;
	m_writing_flash = false;
	m_write_resetting = false;
	m_flash_mode = READ_MODE;
	m_flash_status = 0;
	ws_rom_sram_device::device_reset();
}

void ws_rom_eeprom_device::device_reset()
{
	m_eeprom_address = 0;
	m_eeprom_command = 0;
	m_eeprom_start = 0;
	m_eeprom_write_enabled = 0;
	switch (m_nvram.size())
	{
		case 0x80:
			m_eeprom_mode = EEPROM_1K;
			break;
		case 0x400:
			m_eeprom_mode = EEPROM_8K;
			break;
		case 0x800:
			m_eeprom_mode = EEPROM_16K;
			break;
	}
	ws_rom_device::device_reset();
}



//-------------------------------------------------
//  device_timer - handler timer events
//-------------------------------------------------

void ws_rom_device::device_timer(emu_timer &timer, device_timer_id id, int param)
{
	if (id == TIMER_RTC)
	{
		// a second passed
		m_rtc_second++;
		if ((m_rtc_second & 0x0f) > 9)
			m_rtc_second = (m_rtc_second & 0xf0) + 0x10;

		// check for minute passed
		if (m_rtc_second >= 0x60)
		{
			m_rtc_second = 0;
			m_rtc_minute++;
			if ((m_rtc_minute & 0x0f) > 9)
				m_rtc_minute = (m_rtc_minute & 0xf0) + 0x10;
		}

		// check for hour passed
		if (m_rtc_minute >= 0x60)
		{
			m_rtc_minute = 0;
			m_rtc_hour++;
			if ((m_rtc_hour & 0x0f) > 9)
				m_rtc_hour = (m_rtc_hour & 0xf0) + 0x10;
			if (m_rtc_hour == 0x12)
				m_rtc_hour |= 0x80;
		}

		// check for day passed
		if (m_rtc_hour >= 0x24)
		{
			m_rtc_hour = 0;
			m_rtc_day++;
		}
	}
}


/*-------------------------------------------------
 mapper specific handlers
 -------------------------------------------------*/

u16 ws_rom_device::read_rom20(offs_t offset, u16 mem_mask)
{
	return m_rom[offset + m_base20];
}


u16 ws_rom_device::read_rom30(offs_t offset, u16 mem_mask)
{
	return m_rom[offset + m_base30];
}


u16 ws_rom_device::read_rom40(offs_t offset, u16 mem_mask)
{
	// we still need to mask in some cases, e.g. when game is 512K
	return m_rom[(offset + m_base40) & m_rom_mask];
}


u16 ws_rom_device::read_io(offs_t offset, u16 mem_mask)
{
	u16 value = m_io_regs[offset];

	switch (offset)
	{
		case 0x0a / 2:
			if (!m_has_rtc)
				break;

			// RTC data
			if (ACCESSING_BITS_8_15)
			{
				if ((m_io_regs[0x0a / 2] & 0xff) == 0x95 && (m_rtc_index < 7))
				{
					switch (m_rtc_index)
					{
						case 0: value = (value & 0xff) | (m_rtc_year << 8); break;
						case 1: value = (value & 0xff) | (m_rtc_month << 8); break;
						case 2: value = (value & 0xff) | (m_rtc_day << 8); break;
						case 3: value = (value & 0xff) | (m_rtc_day_of_week << 8); break;
						case 4: value = (value & 0xff) | (m_rtc_hour << 8); break;
						case 5: value = (value & 0xff) | (m_rtc_minute << 8); break;
						case 6: value = (value & 0xff) | (m_rtc_second << 8); break;
					}
					m_rtc_index++;
				}
			}
			break;
	}

	return value;
}

void ws_rom_device::write_io(offs_t offset, u16 data, u16 mem_mask)
{
	switch (offset)
	{
		case 0x00 / 2:
			// Bit 0-3 - ROM bank base register for segments 4-15
			// Bit 4-7 - Unknown
			if (ACCESSING_BITS_0_7)
			{
				m_base40 = (((((data & 0x0f) << 4) | 4) & m_bank_mask) << 15) & m_rom_mask;
			}
			break;
		case 0x02 / 2:
			// ROM bank for segment 2 (0x20000 - 0x2ffff)
			if (ACCESSING_BITS_0_7)
			{
				m_base20 = (((data & 0xff) & m_bank_mask) << 15) & m_rom_mask;
			}
			// ROM bank for segment 3 (0x30000 - 0x3ffff)
			if (ACCESSING_BITS_8_15)
			{
				m_base30 = (((data >> 8) & m_bank_mask) << 15) & m_rom_mask;
			}
			break;
		case 0x0a / 2:
			if (!m_has_rtc)
				break;
			// RTC Command
			// Bit 0-4 - RTC command
			//   10000 - Reset
			//   10010 - Write timer settings (alarm)
			//   10011 - Read timer settings (alarm)
			//   10100 - Set time/date
			//   10101 - Get time/date
			// Bit 5-6 - Unknown
			// Bit 7   - Command done (read only)
			if (ACCESSING_BITS_0_7)
			{
				switch (data & 0xff)
				{
					case 0x10:  // Reset
						m_rtc_index = 8;
						m_rtc_year = 0;
						m_rtc_month = 1;
						m_rtc_day = 1;
						m_rtc_day_of_week = 0;
						m_rtc_hour = 0;
						m_rtc_minute = 0;
						m_rtc_second = 0;
						m_rtc_setting = 0xff;
						data |= 0x80;
						break;
					case 0x12:  // Write Timer Settings (Alarm)
						m_rtc_index = 8;
						m_rtc_setting = m_io_regs[0x0a / 2] >> 8;
						data |= 0x80;
						break;
					case 0x13:  // Read Timer Settings (Alarm)
						m_rtc_index = 8;
						m_io_regs[0x0a / 2] = (m_io_regs[0x0a / 2] & 0x00ff) | (m_rtc_setting << 8);
						data |= 0x80;
						break;
					case 0x14:  // Set Time/Date
						m_rtc_year = m_io_regs[0x0a / 2] >> 8;
						m_rtc_index = 1;
						data |= 0x80;
						break;
					case 0x15:  // Get Time/Date
						m_rtc_index = 0;
						data |= 0x80;
						m_io_regs[0x0a / 2] = (m_io_regs[0x0a / 2] & 0x00ff) | (m_rtc_year << 8);
						break;
					default:
						logerror( "Unknown RTC command (%X) requested\n", data & 0xff);
				}
			}
			// RTC Data
			if (ACCESSING_BITS_8_15)
			{
				if ((m_io_regs[0x0a / 2] & 0xff) == 0x94 && m_rtc_index < 7)
				{
					switch (m_rtc_index)
					{
						case 0: m_rtc_year = data >> 8; break;
						case 1: m_rtc_month = data >> 8; break;
						case 2: m_rtc_day = data >> 8; break;
						case 3: m_rtc_day_of_week = data >> 8; break;
						case 4: m_rtc_hour = data >> 8; break;
						case 5: m_rtc_minute = data >> 8; break;
						case 6: m_rtc_second = data >> 8; break;
					}
					m_rtc_index++;
				}
			}
			break;
	}

	COMBINE_DATA(&m_io_regs[offset]);
}

u16 ws_rom_sram_device::read_ram(offs_t offset, u16 mem_mask)
{
	u32 nvram_address = (m_nvram_base + (offset << 1)) & (m_nvram_size - 1);
	return m_nvram[nvram_address] | (m_nvram[nvram_address + 1] << 8);
}

void ws_rom_sram_device::write_ram(offs_t offset, u16 data, u16 mem_mask)
{
	u32 nvram_address = (m_nvram_base + (offset << 1)) & (m_nvram_size - 1);
	if (ACCESSING_BITS_0_7)
		m_nvram[nvram_address] = data & 0xff;
	if (ACCESSING_BITS_8_15)
		m_nvram[nvram_address + 1] = data >> 8;
}

void ws_rom_sram_device::write_io(offs_t offset, u16 data, u16 mem_mask)
{
	switch (offset)
	{
		case 0x00 / 2:
			// SRAM bank to select
			if (ACCESSING_BITS_8_15)
			{
				m_nvram_base = ((data >> 8) * 0x10000) & (m_nvram.size() -  1);
			}
			[[fallthrough]];
		default:
			ws_rom_device::write_io(offset, data, mem_mask);
			break;
	}
}


u16 ws_rom_eeprom_device::read_io(offs_t offset, u16 mem_mask)
{
	u16 value = m_io_regs[offset];

	switch (offset)
	{
		case 0x04 / 2:
		case 0x06 / 2:
		case 0x08 / 2:
			// EEPROM reads, taken from regs
			break;
		default:
			value = ws_rom_device::read_io(offset, mem_mask);
			break;
	}

	return value;
}

void ws_rom_eeprom_device::write_io(offs_t offset, u16 data, u16 mem_mask)
{
	switch (offset)
	{
		case 0x06 / 2:
			/* EEPROM address lower bits port/EEPROM address and command port
			         1KBit EEPROM:
			         Bit 0-5 - EEPROM address bit 1-6
			         Bit 6-7 - Command
			         00 - Extended command address bit 4-5:
			         00 - Write disable
			         01 - Write all
			         10 - Erase all
			         11 - Write enable
			         01 - Write
			         10 - Read
			         11 - Erase
			         16KBit EEPROM:
			         Bit 0-7 - EEPROM address bit 1-8
			         */
			if (ACCESSING_BITS_0_7)
			{
				switch (m_eeprom_mode)
				{
					case EEPROM_1K:
						m_eeprom_address = data & 0x3f;
						m_eeprom_command = (data >> 4) & 0x0f;
						if ((m_eeprom_command & 0x0c) != 0x00)
							m_eeprom_command = m_eeprom_command & 0x0c;
						break;

					case EEPROM_8K:
					case EEPROM_16K:
						m_eeprom_address = (m_eeprom_address & 0xff00) | (data & 0xff);
						break;

					default:
						logerror( "Write EEPROM address/register register C6 for unsupported EEPROM type\n" );
						break;
				}
			}
			/* EEPROM higher bits/command bits port
			         1KBit EEPROM:
			         Bit 0   - Start
			         Bit 1-7 - Unknown
			         16KBit EEPROM:
			         Bit 0-1 - EEPROM address bit 9-10
			         Bit 2-3 - Command
			         00 - Extended command address bit 0-1:
			         00 - Write disable
			         01 - Write all
			         10 - Erase all
			         11 - Write enable
			         01 - Write
			         10 - Read
			         11 - Erase
			         Bit 4   - Start
			         Bit 5-7 - Unknown
			         */
			if (ACCESSING_BITS_8_15)
			{
				switch (m_eeprom_mode)
				{
					case EEPROM_1K:
						m_eeprom_start = (data >> 8) & 0x01;
						break;

					case EEPROM_8K:
						m_eeprom_address = (data & 0x0100) | (m_eeprom_address & 0xff);
						m_eeprom_command = (data >> 8) & 0x0f;
						if ((m_eeprom_command & 0x0c) != 0x00)
							m_eeprom_command = m_eeprom_command & 0x0c;
						m_eeprom_start = (data >> 12) & 0x01;
						break;

					case EEPROM_16K:
						m_eeprom_address = (data & 0x0300) | (m_eeprom_address & 0xff);
						m_eeprom_command = (data >> 8) & 0x0f;
						if ((m_eeprom_command & 0x0c) != 0x00)
							m_eeprom_command = m_eeprom_command & 0x0c;
						m_eeprom_start = (data >> 12) & 0x01;
						break;

					default:
						logerror( "Write EEPROM address/command register C7 for unsupported EEPROM type\n" );
					break;
				}
			}
			break;

		case 0x08 / 2:
			/* EEPROM command
			         Bit 0   - Read complete (read only)
			         Bit 1   - Write complete (read only)
			         Bit 2-3 - Unknown
			         Bit 4   - Read
			         Bit 5   - Write
			         Bit 6   - Protect
			         Bit 7   - Initialize
			         */
			if (ACCESSING_BITS_0_7)
			{
				if (data & 0x80)    // Initialize
					logerror("Unsupported EEPROM command 'Initialize'\n");

				if (data & 0x40)    // Protect
				{
					switch (m_eeprom_command)
					{
						case 0x00:
							m_eeprom_write_enabled = 0;
							data |= 0x02;
							break;
						case 0x03:
							m_eeprom_write_enabled = 1;
							data |= 0x02;
							break;
						default:
							logerror("Unsupported 'Protect' command %X\n", m_eeprom_command);
							break;
					}
				}

				if (data & 0x20)    // Write
				{
					if (m_eeprom_write_enabled)
					{
						switch (m_eeprom_command)
						{
							case 0x04:
								m_nvram[(m_eeprom_address << 1) + 1] = m_io_regs[0x04 / 2] & 0xff;
								m_nvram[m_eeprom_address << 1] = m_io_regs[0x04 / 2] >> 8;
								data |= 0x02;
								break;
							default:
								logerror("Unsupported 'Write' command %X\n", m_eeprom_command);
								break;
						}
					}
				}

				if (data & 0x10)    // Read
				{
					m_io_regs[0x04] = m_nvram[(m_eeprom_address << 1) + 1];
					m_io_regs[0x05] = m_nvram[m_eeprom_address << 1];
					data |= 0x01;
				}
			}
			break;

		default:
			ws_rom_device::write_io(offset, data, mem_mask);
			break;
	}

	COMBINE_DATA(&m_io_regs[offset]);
}


u16 ws_wwitch_device::read_ram(offs_t offset, u16 mem_mask)
{
	if (m_flash_mode == COMMAND_MODE)
	{
		if (!machine().side_effects_disabled())
		{
			if (m_writing_flash)
			{
				m_flash_status ^= 0x40;
				m_flash_count++;
				if (m_flash_count > 4) {
					m_writing_flash = false;
					m_flash_mode = READ_MODE;
				}
			}
			// After initiating an erase block command the wwitch expects to see bit 7 set
			if (m_flash_command == 0x30)
			{
				m_flash_status |= 0x80;
				m_flash_mode = READ_MODE;
			}
		}
		return m_flash_status;
	}
	if (m_io_regs[0x01] >= 8 && m_io_regs[0x01] < 16)
	{
		return m_rom[((m_io_regs[0x01] * 0x8000) | offset) & m_rom_mask];
	}
	if (m_io_regs[0x01] < 8)
	{
		return ws_rom_sram_device::read_ram(offset, mem_mask);
	}
	else
	{
		return 0xffff;
	}
}


void ws_wwitch_device::write_ram(offs_t offset, u16 data, u16 mem_mask)
{
	if (m_flash_seq == 0 && offset == (0xaaa >> 1) && ACCESSING_BITS_0_7 && data == 0xaa)
	{
		m_flash_seq = 1;
	}
	else if (m_flash_seq == 1 && offset == (0x555 >> 1) && ACCESSING_BITS_8_15 && data == 0x55)
	{
		m_flash_seq = 2;
	}
	else if (m_flash_seq == 2 && offset == (0xaaa >> 1) && ACCESSING_BITS_0_7)
	{
		switch (data)
		{
		case 0x10:  // Chip erase
			if (m_flash_command == 0x80)
			{
				// TODO
			}
			break;
		case 0x20:  // Set to fast mode
			m_flash_command = data;
			m_flash_mode = COMMAND_MODE;
			break;
		case 0x30:  // (Erase) block
			// TODO: Any write to a block address triggers the block erase
			if (m_flash_command == 0x80)
			{
				if ((m_io_regs[0x01] & 0x07) < 7)
				{
						u32 block_base = (m_io_regs[0x01] & 0x07) << 15;
						for (u32 address = 0; address < 0x8000; address++)
						{
							m_rom[(block_base | address) & m_rom_mask] = 0xffff;
						}
				}

				m_flash_command = data;
				m_flash_mode = COMMAND_MODE;
			}
			break;
		case 0x80:  // Erase (chip or block)
			m_flash_command = data;
			break;
		default:    // Unknown command
			m_flash_command = 0;
		}
		m_flash_seq = 0;
	}
	else if (m_io_regs[0x01] >= 8 && m_io_regs[0x01] < 15)
	{
		if (m_write_flash)
		{
				// perform write
				if (!ACCESSING_BITS_0_7)
					data |= 0xff;
				if (!ACCESSING_BITS_8_15)
					data |= 0xff00;
				m_rom[((m_io_regs[0x01] * 0x8000) | offset) & m_rom_mask] &= data;
				m_flash_status = (m_flash_status & 0x7f) | ((data ^ 0x80) & 0x80);
				m_writing_flash = true;
				m_flash_count = 0;
				m_write_flash = false;
		}
		else if (m_flash_command == 0x20)
		{
			switch (data)
			{
			case 0x00:
				if (m_write_resetting)
				{
					m_write_flash = false;
					m_write_resetting = false;
					m_flash_mode = READ_MODE;
				}
				break;
			case 0xF0:    // Reset from fast mode #2
				m_write_flash = false;
				m_write_resetting = false;
				m_flash_mode = READ_MODE;
				break;
			case 0x90:    // Reset from fast mode #1
				m_write_resetting = true;
				break;
			case 0xA0:    // Fast program
				if (m_flash_command == 0x20)
				{
					m_write_flash = true;
					m_flash_mode = COMMAND_MODE;
				}
				break;
			}
		}
	}
	else if (m_io_regs[0x01] < 8)
	{
		ws_rom_sram_device::write_ram(offset, data, mem_mask);
	}
}
