// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/**********************************************************************

    DALLAS DS2404

    RTC + BACKUP RAM

**********************************************************************/

#include "emu.h"
#include "ds2404.h"
#include <time.h>


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

// device type definition
const device_type DS2404 = &device_creator<ds2404_device>;

//-------------------------------------------------
//  ds2404_device - constructor
//-------------------------------------------------

ds2404_device::ds2404_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, DS2404, "DS2404", tag, owner, clock, "ds2404", __FILE__),
		device_nvram_interface(mconfig, *this), m_tick_timer(nullptr), m_ref_year(0), m_ref_month(0), m_ref_day(0),
		m_address(0),
		m_offset(0),
		m_end_offset(0),
		m_a1(0),
		m_a2(0),
		m_state_ptr(0)
{
	memset(m_ram, 0, sizeof(m_ram));
}


//-------------------------------------------------
//  static_set_ref_year - configuration helper
//  to set the reference year
//-------------------------------------------------

void ds2404_device::static_set_ref_year(device_t &device, UINT32 year)
{
	ds2404_device &ds2404 = downcast<ds2404_device &>(device);
	ds2404.m_ref_year = year;
}


//-------------------------------------------------
//  static_set_ref_month - configuration helper
//  to set the reference month
//-------------------------------------------------

void ds2404_device::static_set_ref_month(device_t &device, UINT8 month)
{
	ds2404_device &ds2404 = downcast<ds2404_device &>(device);
	ds2404.m_ref_month = month;
}


//-------------------------------------------------
//  static_set_ref_day - configuration helper
//  to set the reference day
//-------------------------------------------------

void ds2404_device::static_set_ref_day(device_t &device, UINT8 day)
{
	ds2404_device &ds2404 = downcast<ds2404_device &>(device);
	ds2404.m_ref_day = day;
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
		elem = DS2404_STATE_IDLE;

	m_tick_timer = timer_alloc(0);
	m_tick_timer->adjust(attotime::from_hz(256), 0, attotime::from_hz(256));
}


void ds2404_device::ds2404_rom_cmd(UINT8 cmd)
{
	switch(cmd)
	{
		case 0xcc:      /* Skip ROM */
			m_state[0] = DS2404_STATE_COMMAND;
			m_state_ptr = 0;
			break;

		default:
			fatalerror("DS2404: Unknown ROM command %02X\n", cmd);
	}
}

void ds2404_device::ds2404_cmd(UINT8 cmd)
{
	switch(cmd)
	{
		case 0x0f:      /* Write scratchpad */
			m_state[0] = DS2404_STATE_ADDRESS1;
			m_state[1] = DS2404_STATE_ADDRESS2;
			m_state[2] = DS2404_STATE_INIT_COMMAND;
			m_state[3] = DS2404_STATE_WRITE_SCRATCHPAD;
			m_state_ptr = 0;
			break;

		case 0x55:      /* Copy scratchpad */
			m_state[0] = DS2404_STATE_ADDRESS1;
			m_state[1] = DS2404_STATE_ADDRESS2;
			m_state[2] = DS2404_STATE_OFFSET;
			m_state[3] = DS2404_STATE_INIT_COMMAND;
			m_state[4] = DS2404_STATE_COPY_SCRATCHPAD;
			m_state_ptr = 0;
			break;

		case 0xf0:      /* Read memory */
			m_state[0] = DS2404_STATE_ADDRESS1;
			m_state[1] = DS2404_STATE_ADDRESS2;
			m_state[2] = DS2404_STATE_INIT_COMMAND;
			m_state[3] = DS2404_STATE_READ_MEMORY;
			m_state_ptr = 0;
			break;

		default:
			fatalerror("DS2404: Unknown command %02X\n", cmd);
	}
}

UINT8 ds2404_device::ds2404_readmem()
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

void ds2404_device::ds2404_writemem(UINT8 value)
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

WRITE8_MEMBER( ds2404_device::ds2404_1w_reset_w )
{
	m_state[0] = DS2404_STATE_IDLE;
	m_state_ptr = 0;
}

WRITE8_MEMBER( ds2404_device::ds2404_3w_reset_w )
{
	m_state[0] = DS2404_STATE_COMMAND;
	m_state_ptr = 0;
}

READ8_MEMBER( ds2404_device::ds2404_data_r )
{
	UINT8 value = 0;
	switch(m_state[m_state_ptr])
	{
		case DS2404_STATE_IDLE:
		case DS2404_STATE_COMMAND:
		case DS2404_STATE_ADDRESS1:
		case DS2404_STATE_ADDRESS2:
		case DS2404_STATE_OFFSET:
		case DS2404_STATE_INIT_COMMAND:
			break;

		case DS2404_STATE_READ_MEMORY:
			value = ds2404_readmem();
			break;

		case DS2404_STATE_READ_SCRATCHPAD:
			if(m_offset < 0x20)
			{
				value = m_ram[m_offset];
				m_offset++;
			}
			break;

		case DS2404_STATE_WRITE_SCRATCHPAD:
			break;

		case DS2404_STATE_COPY_SCRATCHPAD:
			break;
	}
	return value;
}

WRITE8_MEMBER( ds2404_device::ds2404_data_w )
{
	switch( m_state[m_state_ptr] )
	{
		case DS2404_STATE_IDLE:
			ds2404_rom_cmd(data & 0xff);
			break;

		case DS2404_STATE_COMMAND:
			ds2404_cmd(data & 0xff);
			break;

		case DS2404_STATE_ADDRESS1:
			m_a1 = data & 0xff;
			m_state_ptr++;
			break;

		case DS2404_STATE_ADDRESS2:
			m_a2 = data & 0xff;
			m_state_ptr++;
			break;

		case DS2404_STATE_OFFSET:
			m_end_offset = data & 0xff;
			m_state_ptr++;
			break;

		case DS2404_STATE_INIT_COMMAND:
			break;

		case DS2404_STATE_READ_MEMORY:
			break;

		case DS2404_STATE_READ_SCRATCHPAD:
			break;

		case DS2404_STATE_WRITE_SCRATCHPAD:
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

		case DS2404_STATE_COPY_SCRATCHPAD:
			break;
	}

	if( m_state[m_state_ptr] == DS2404_STATE_INIT_COMMAND )
	{
		switch( m_state[m_state_ptr + 1] )
		{
			case DS2404_STATE_IDLE:
			case DS2404_STATE_COMMAND:
			case DS2404_STATE_ADDRESS1:
			case DS2404_STATE_ADDRESS2:
			case DS2404_STATE_OFFSET:
			case DS2404_STATE_INIT_COMMAND:
				break;

			case DS2404_STATE_READ_MEMORY:
				m_address = (m_a2 << 8) | m_a1;
				m_address -= 1;
				break;

			case DS2404_STATE_WRITE_SCRATCHPAD:
				m_address = (m_a2 << 8) | m_a1;
				m_offset = m_address & 0x1f;
				break;

			case DS2404_STATE_READ_SCRATCHPAD:
				m_address = (m_a2 << 8) | m_a1;
				m_offset = m_address & 0x1f;
				break;

			case DS2404_STATE_COPY_SCRATCHPAD:
				m_address = (m_a2 << 8) | m_a1;

				for(int i = 0; i <= m_end_offset; i++)
				{
					ds2404_writemem(m_ram[i]);
					m_address++;
				}
				break;
		}
		m_state_ptr++;
	}
}

WRITE8_MEMBER( ds2404_device::ds2404_clk_w )
{
	switch( m_state[m_state_ptr] )
	{
		case DS2404_STATE_IDLE:
		case DS2404_STATE_COMMAND:
		case DS2404_STATE_ADDRESS1:
		case DS2404_STATE_ADDRESS2:
		case DS2404_STATE_OFFSET:
		case DS2404_STATE_INIT_COMMAND:
			break;

		case DS2404_STATE_READ_MEMORY:
			m_address++;
			break;

		case DS2404_STATE_READ_SCRATCHPAD:
			break;

		case DS2404_STATE_WRITE_SCRATCHPAD:
			break;

		case DS2404_STATE_COPY_SCRATCHPAD:
			break;
	}
}

void ds2404_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch(id)
	{
		case 0:
		{
			// tick
			for(auto & elem : m_rtc)
			{
				elem++;
				if(elem != 0)
				{
					break;
				}
			}

			break;
		}

		default:
			assert_always(FALSE, "Unknown id in ds2404_device::device_timer");
			break;
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

void ds2404_device::nvram_read(emu_file &file)
{
	file.read(m_sram, sizeof(m_sram));
}


//-------------------------------------------------
//  nvram_write - called to write NVRAM to the
//  .nv file
//-------------------------------------------------

void ds2404_device::nvram_write(emu_file &file)
{
	file.write(m_sram, sizeof(m_sram));
}
