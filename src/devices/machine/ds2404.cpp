// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/**********************************************************************

    DALLAS DS2404

    RTC + BACKUP RAM

    FIXME: This device implements a parallel interface provided by the
    Seibu SEI600 for the convenience of SPI emulation. It does not
    implement the 1-Wire protocol actually used with that ASIC, nor
    does it implement the alternate 3-Wire serial interface.

**********************************************************************/

#include "emu.h"
#include "ds2404.h"

#include <algorithm>
#include <ctime> // FIXME: re-write in terms of device_rtc_interface and remove this


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

// device type definition
DEFINE_DEVICE_TYPE(DS2404, ds2404_device, "ds2404", "DS2404 EconoRAM Time Chip")

//-------------------------------------------------
//  ds2404_device - constructor
//-------------------------------------------------

ds2404_device::ds2404_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, DS2404, tag, owner, clock),
		device_nvram_interface(mconfig, *this), m_tick_timer(nullptr), m_ref_year(0), m_ref_month(0), m_ref_day(0),
		m_address(0),
		m_offset(0),
		m_end_offset(0),
		m_a1(0),
		m_a2(0),
		m_state_ptr(0)
{
	std::fill(std::begin(m_ram), std::end(m_ram), 0);
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void ds2404_device::device_start()
{
	struct tm ref_tm;

	memset(&ref_tm, 0, sizeof(ref_tm));
	ref_tm.tm_year = m_ref_year - 1900;
	ref_tm.tm_mon = m_ref_month - 1;
	ref_tm.tm_mday = m_ref_day;

	time_t ref_time = mktime(&ref_tm);

	time_t current_time;
	time(&current_time);
	current_time -= ref_time;

	m_rtc[0] = 0x0;
	m_rtc[1] = (current_time >> 0) & 0xff;
	m_rtc[2] = (current_time >> 8) & 0xff;
	m_rtc[3] = (current_time >> 16) & 0xff;
	m_rtc[4] = (current_time >> 24) & 0xff;

	for (auto & elem : m_state)
		elem = STATE_IDLE;

	m_tick_timer = timer_alloc(0);
	m_tick_timer->adjust(attotime::from_hz(256), 0, attotime::from_hz(256));
}


void ds2404_device::rom_cmd(uint8_t cmd)
{
	switch(cmd)
	{
		case 0xcc:      /* Skip ROM */
			m_state[0] = STATE_COMMAND;
			m_state_ptr = 0;
			break;

		default:
			fatalerror("DS2404: Unknown ROM command %02X\n", cmd);
	}
}

void ds2404_device::cmd(uint8_t cmd)
{
	switch(cmd)
	{
		case 0x0f:      /* Write scratchpad */
			m_state[0] = STATE_ADDRESS1;
			m_state[1] = STATE_ADDRESS2;
			m_state[2] = STATE_INIT_COMMAND;
			m_state[3] = STATE_WRITE_SCRATCHPAD;
			m_state_ptr = 0;
			break;

		case 0x55:      /* Copy scratchpad */
			m_state[0] = STATE_ADDRESS1;
			m_state[1] = STATE_ADDRESS2;
			m_state[2] = STATE_OFFSET;
			m_state[3] = STATE_INIT_COMMAND;
			m_state[4] = STATE_COPY_SCRATCHPAD;
			m_state_ptr = 0;
			break;

		case 0xf0:      /* Read memory */
			m_state[0] = STATE_ADDRESS1;
			m_state[1] = STATE_ADDRESS2;
			m_state[2] = STATE_INIT_COMMAND;
			m_state[3] = STATE_READ_MEMORY;
			m_state_ptr = 0;
			break;

		default:
			fatalerror("DS2404: Unknown command %02X\n", cmd);
	}
}

uint8_t ds2404_device::readmem()
{
	if( m_address < 0x200 )
	{
		return m_sram[ m_address ];
	}
	else if( m_address >= 0x202 && m_address <= 0x206 )
	{
		return m_rtc[ m_address - 0x202 ];
	}
	return 0;
}

void ds2404_device::writemem(uint8_t value)
{
	if( m_address < 0x200 )
	{
		m_sram[ m_address ] = value;
	}
	else if( m_address >= 0x202 && m_address <= 0x206 )
	{
		m_rtc[ m_address - 0x202 ] = value;
	}
}

void ds2404_device::_1w_reset_w(uint8_t data)
{
	m_state[0] = STATE_IDLE;
	m_state_ptr = 0;
}

void ds2404_device::_3w_reset_w(uint8_t data)
{
	m_state[0] = STATE_COMMAND;
	m_state_ptr = 0;
}

uint8_t ds2404_device::data_r()
{
	uint8_t value = 0;
	switch(m_state[m_state_ptr])
	{
		case STATE_IDLE:
		case STATE_COMMAND:
		case STATE_ADDRESS1:
		case STATE_ADDRESS2:
		case STATE_OFFSET:
		case STATE_INIT_COMMAND:
			break;

		case STATE_READ_MEMORY:
			value = readmem();
			break;

		case STATE_READ_SCRATCHPAD:
			if(m_offset < 0x20)
			{
				value = m_ram[m_offset];
				m_offset++;
			}
			break;

		case STATE_WRITE_SCRATCHPAD:
			break;

		case STATE_COPY_SCRATCHPAD:
			break;
	}
	return value;
}

void ds2404_device::data_w(uint8_t data)
{
	switch( m_state[m_state_ptr] )
	{
		case STATE_IDLE:
			rom_cmd(data & 0xff);
			break;

		case STATE_COMMAND:
			cmd(data & 0xff);
			break;

		case STATE_ADDRESS1:
			m_a1 = data & 0xff;
			m_state_ptr++;
			break;

		case STATE_ADDRESS2:
			m_a2 = data & 0xff;
			m_state_ptr++;
			break;

		case STATE_OFFSET:
			m_end_offset = data & 0xff;
			m_state_ptr++;
			break;

		case STATE_INIT_COMMAND:
			break;

		case STATE_READ_MEMORY:
			break;

		case STATE_READ_SCRATCHPAD:
			break;

		case STATE_WRITE_SCRATCHPAD:
			if( m_offset < 0x20 )
			{
				m_ram[m_offset] = data & 0xff;
				m_offset++;
			}
			else
			{
				/* Set OF flag */
			}
			break;

		case STATE_COPY_SCRATCHPAD:
			break;
	}

	if( m_state[m_state_ptr] == STATE_INIT_COMMAND )
	{
		switch( m_state[m_state_ptr + 1] )
		{
			case STATE_IDLE:
			case STATE_COMMAND:
			case STATE_ADDRESS1:
			case STATE_ADDRESS2:
			case STATE_OFFSET:
			case STATE_INIT_COMMAND:
				break;

			case STATE_READ_MEMORY:
				m_address = (m_a2 << 8) | m_a1;
				m_address -= 1;
				break;

			case STATE_WRITE_SCRATCHPAD:
				m_address = (m_a2 << 8) | m_a1;
				m_offset = m_address & 0x1f;
				break;

			case STATE_READ_SCRATCHPAD:
				m_address = (m_a2 << 8) | m_a1;
				m_offset = m_address & 0x1f;
				break;

			case STATE_COPY_SCRATCHPAD:
				m_address = (m_a2 << 8) | m_a1;

				for(int i = 0; i <= m_end_offset; i++)
				{
					writemem(m_ram[i]);
					m_address++;
				}
				break;
		}
		m_state_ptr++;
	}
}

void ds2404_device::clk_w(uint8_t data)
{
	switch( m_state[m_state_ptr] )
	{
		case STATE_IDLE:
		case STATE_COMMAND:
		case STATE_ADDRESS1:
		case STATE_ADDRESS2:
		case STATE_OFFSET:
		case STATE_INIT_COMMAND:
			break;

		case STATE_READ_MEMORY:
			m_address++;
			break;

		case STATE_READ_SCRATCHPAD:
			break;

		case STATE_WRITE_SCRATCHPAD:
			break;

		case STATE_COPY_SCRATCHPAD:
			break;
	}
}

void ds2404_device::device_timer(emu_timer &timer, device_timer_id id, int param)
{
	switch(id)
	{
		case 0:
			// tick
			for(auto &elem : m_rtc)
			{
				elem++;
				if(elem != 0)
					break;
			}
			break;

		default:
			throw emu_fatalerror("Unknown id in ds2404_device::device_timer");
	}
}


//-------------------------------------------------
//  nvram_default - called to initialize NVRAM to
//  its default state
//-------------------------------------------------

void ds2404_device::nvram_default()
{
	memset(m_sram, 0, sizeof(m_sram));
}


//-------------------------------------------------
//  nvram_read - called to read NVRAM from the
//  .nv file
//-------------------------------------------------

bool ds2404_device::nvram_read(util::read_stream &file)
{
	size_t actual;
	return !file.read(m_sram, sizeof(m_sram), actual) && actual == sizeof(m_sram);
}


//-------------------------------------------------
//  nvram_write - called to write NVRAM to the
//  .nv file
//-------------------------------------------------

bool ds2404_device::nvram_write(util::write_stream &file)
{
	size_t actual;
	return !file.write(m_sram, sizeof(m_sram), actual) && actual == sizeof(m_sram);
}
