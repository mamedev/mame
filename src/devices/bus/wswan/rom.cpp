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

const device_type WS_ROM_STD = &device_creator<ws_rom_device>;
const device_type WS_ROM_SRAM = &device_creator<ws_rom_sram_device>;
const device_type WS_ROM_EEPROM = &device_creator<ws_rom_eeprom_device>;


ws_rom_device::ws_rom_device(const machine_config &mconfig, device_type type, std::string name, std::string tag, device_t *owner, UINT32 clock, std::string shortname, std::string source) :
	device_t(mconfig, type, name, tag, owner, clock, shortname, source),
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

ws_rom_device::ws_rom_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
					: device_t(mconfig, WS_ROM_STD, "Wonderswan Standard Carts", tag, owner, clock, "ws_rom", __FILE__),
						device_ws_cart_interface( mconfig, *this ), m_base20(0),
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

ws_rom_sram_device::ws_rom_sram_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
					: ws_rom_device(mconfig, WS_ROM_SRAM, "Wonderswan Carts w/SRAM", tag, owner, clock, "ws_sram", __FILE__), m_nvram_base(0)
				{
}


ws_rom_eeprom_device::ws_rom_eeprom_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
					: ws_rom_device(mconfig, WS_ROM_EEPROM, "Wonderswan Carts w/EEPROM", tag, owner, clock, "ws_eeprom", __FILE__), m_eeprom_mode(0),
	m_eeprom_address(0), m_eeprom_command(0), m_eeprom_start(0), m_eeprom_write_enabled(0)
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
}

void ws_rom_device::device_reset()
{
	m_base20 = ((0xff & m_bank_mask) << 16) & (m_rom_size - 1);
	m_base30 = ((0xff & m_bank_mask) << 16) & (m_rom_size - 1);
	m_base40 = (((0xf0 & m_bank_mask) | 4) << 16) & (m_rom_size - 1);

	memset(m_io_regs, 0, sizeof(m_io_regs));

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

void ws_rom_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
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

READ8_MEMBER(ws_rom_device::read_rom20)
{
	return m_rom[offset + m_base20];
}


READ8_MEMBER(ws_rom_device::read_rom30)
{
	return m_rom[offset + m_base30];
}


READ8_MEMBER(ws_rom_device::read_rom40)
{
	// we still need to mask in some cases, e.g. when game is 512K
	return m_rom[(offset + m_base40) & (m_rom_size - 1)];
}


READ8_MEMBER(ws_rom_device::read_io)
{
	UINT8 value = m_io_regs[offset];

	switch (offset)
	{
		case 0x0b:      // RTC data
			if (!m_has_rtc)
				break;

			if (m_io_regs[0x0a] == 0x95 && (m_rtc_index < 7))
			{
				switch (m_rtc_index)
				{
					case 0: value = m_rtc_year; break;
					case 1: value = m_rtc_month; break;
					case 2: value = m_rtc_day; break;
					case 3: value = m_rtc_day_of_week; break;
					case 4: value = m_rtc_hour; break;
					case 5: value = m_rtc_minute; break;
					case 6: value = m_rtc_second; break;
				}
				m_rtc_index++;
			}
			break;
	}

	return value;
}

WRITE8_MEMBER(ws_rom_device::write_io)
{
	switch (offset)
	{
		case 0x00:
			// Bit 0-3 - ROM bank base register for segments 3-15
			// Bit 4-7 - Unknown
			data = ((data & 0x0f) << 4) | 4;
			m_base40 = ((data & m_bank_mask) << 16) & (m_rom_size - 1);
			break;
		case 0x02: // ROM bank for segment 2 (0x20000 - 0x2ffff)
			m_base20 = ((data & m_bank_mask) << 16) & (m_rom_size - 1);
			break;
		case 0x03: // ROM bank for segment 3 (0x30000 - 0x3ffff)
			m_base30 = ((data & m_bank_mask) << 16) & (m_rom_size - 1);
			break;
		case 0x0a:  // RTC Command
					// Bit 0-4 - RTC command
					//   10000 - Reset
					//   10010 - Write timer settings (alarm)
					//   10011 - Read timer settings (alarm)
					//   10100 - Set time/date
					//   10101 - Get time/date
					// Bit 5-6 - Unknown
					// Bit 7   - Command done (read only)
			if (!m_has_rtc)
				break;

			switch (data)
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
					m_rtc_setting = m_io_regs[0x0b];
					data |= 0x80;
					break;
				case 0x13:  // Read Timer Settings (Alarm)
					m_rtc_index = 8;
					m_io_regs[0x0b] = m_rtc_setting;
					data |= 0x80;
					break;
				case 0x14:  // Set Time/Date
					m_rtc_year = m_io_regs[0x0b];
					m_rtc_index = 1;
					data |= 0x80;
					break;
				case 0x15:  // Get Time/Date
					m_rtc_index = 0;
					data |= 0x80;
					m_io_regs[0x0b] = m_rtc_year;
					break;
				default:
					logerror( "Unknown RTC command (%X) requested\n", data);
			}
			break;
		case 0x0b:  // RTC Data
			if (!m_has_rtc)
				break;

			if (m_io_regs[0x0a] == 0x94 && m_rtc_index < 7)
			{
				switch (m_rtc_index)
				{
					case 0: m_rtc_year = data; break;
					case 1: m_rtc_month = data; break;
					case 2: m_rtc_day = data; break;
					case 3: m_rtc_day_of_week = data; break;
					case 4: m_rtc_hour = data; break;
					case 5: m_rtc_minute = data; break;
					case 6: m_rtc_second = data; break;
				}
				m_rtc_index++;
			}
			break;
	}

	m_io_regs[offset] = data;
}

READ8_MEMBER(ws_rom_sram_device::read_ram)
{
	return m_nvram[m_nvram_base + offset];
}

WRITE8_MEMBER(ws_rom_sram_device::write_ram)
{
	m_nvram[m_nvram_base + offset] = data;
}

WRITE8_MEMBER(ws_rom_sram_device::write_io)
{
	switch (offset)
	{
		case 0x01:  // SRAM bank to select
			m_nvram_base = (data * 0x10000) & (m_nvram.size() -  1);
		default:
			ws_rom_device::write_io(space, offset, data);
			break;
	}
}


READ8_MEMBER(ws_rom_eeprom_device::read_io)
{
	UINT8 value = m_io_regs[offset];

	switch (offset)
	{
		case 0x04:
		case 0x05:
		case 0x06:
		case 0x07:
		case 0x08:
			// EEPROM reads, taken from regs
			break;
		default:
			value = ws_rom_device::read_io(space, offset);
			break;
	}

	return value;
}

WRITE8_MEMBER(ws_rom_eeprom_device::write_io)
{
	switch (offset)
	{
		case 0x06:  /* EEPROM address lower bits port/EEPROM address and command port
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
			switch (m_eeprom_mode)
			{
				case EEPROM_1K:
						m_eeprom_address = data & 0x3f;
						m_eeprom_command = data >> 4;
					if ((m_eeprom_command & 0x0c) != 0x00)
						m_eeprom_command = m_eeprom_command & 0x0c;
					break;

				case EEPROM_8K:
				case EEPROM_16K:
					m_eeprom_address = (m_eeprom_address & 0xff00) | data;
					break;

				default:
					logerror( "Write EEPROM address/register register C6 for unsupported EEPROM type\n" );
					break;
			}
			break;

		case 0x07:  /* EEPROM higher bits/command bits port
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
			switch (m_eeprom_mode)
			{
				case EEPROM_1K:
					m_eeprom_start = data & 0x01;
					break;

				case EEPROM_8K:
					m_eeprom_address = ((data & 0x01) << 8) | (m_eeprom_address & 0xff);
					m_eeprom_command = data & 0x0f;
					if ((m_eeprom_command & 0x0c) != 0x00)
						m_eeprom_command = m_eeprom_command & 0x0c;
					m_eeprom_start = (data >> 4) & 0x01;
					break;

				case EEPROM_16K:
					m_eeprom_address = ((data & 0x03) << 8) | (m_eeprom_address & 0xff);
					m_eeprom_command = data & 0x0f;
					if ((m_eeprom_command & 0x0c) != 0x00)
						m_eeprom_command = m_eeprom_command & 0x0c;
					m_eeprom_start = (data >> 4) & 0x01;
					break;

				default:
					logerror( "Write EEPROM address/command register C7 for unsupported EEPROM type\n" );
				break;
			}
			break;

		case 0x08:  /* EEPROM command
                     Bit 0   - Read complete (read only)
                     Bit 1   - Write complete (read only)
                     Bit 2-3 - Unknown
                     Bit 4   - Read
                     Bit 5   - Write
                     Bit 6   - Protect
                     Bit 7   - Initialize
                     */
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
							m_nvram[(m_eeprom_address << 1) + 1] = m_io_regs[0x04];
							m_nvram[m_eeprom_address << 1] = m_io_regs[0x05];
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
			break;

		default:
			ws_rom_device::write_io(space, offset, data);
			break;
	}

	m_io_regs[offset] = data;
}
