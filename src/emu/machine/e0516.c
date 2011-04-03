/**********************************************************************

    E05-16 Real Time Clock emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************/

#include "emu.h"
#include "e0516.h"
#include "machine/devhelpr.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define LOG 1


// states
enum
{
	STATE_ADDRESS = 0,
	STATE_DATA
};


// registers
enum
{
	SECOND = 0,
	MINUTE,
	HOUR,
	DAY,
	MONTH,
	DAY_OF_WEEK,
	YEAR,
	ALL
};



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type E0516 = e0516_device_config::static_alloc_device_config;



//**************************************************************************
//  DEVICE CONFIGURATION
//**************************************************************************

GENERIC_DEVICE_CONFIG_SETUP(e0516, "E05-16")



//**************************************************************************
//  INLINE HELPERS
//**************************************************************************



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  e0516_device - constructor
//-------------------------------------------------

e0516_device::e0516_device(running_machine &_machine, const e0516_device_config &config)
    : device_t(_machine, config),
      m_config(config)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void e0516_device::device_start()
{
	// allocate timers
	m_timer = timer_alloc();
	m_timer->adjust(attotime::from_hz(clock() / 32768), 0, attotime::from_hz(clock() / 32768));

	// state saving
	save_item(NAME(m_cs));
	save_item(NAME(m_clk));
	save_item(NAME(m_data_latch));
	save_item(NAME(m_reg_latch));
	save_item(NAME(m_read_write));
	save_item(NAME(m_state));
	save_item(NAME(m_bits));
	save_item(NAME(m_dio));
	save_item(NAME(m_register));
}


//-------------------------------------------------
//  device_start - device-specific reset
//-------------------------------------------------

void e0516_device::device_reset()
{
}


//-------------------------------------------------
//  device_timer - handler timer events
//-------------------------------------------------

void e0516_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	m_register[SECOND]++;

	if (m_register[SECOND] == 60)
	{
		m_register[SECOND] = 0;
		m_register[MINUTE]++;
	}

	if (m_register[MINUTE] == 60)
	{
		m_register[MINUTE] = 0;
		m_register[HOUR]++;
	}

	if (m_register[HOUR] == 24)
	{
		m_register[HOUR] = 0;
		m_register[DAY]++;
		m_register[DAY_OF_WEEK]++;
	}

	if (m_register[DAY_OF_WEEK] == 8)
	{
		m_register[DAY_OF_WEEK] = 1;
	}

	if (m_register[DAY] == 32)
	{
		m_register[DAY] = 1;
		m_register[MONTH]++;
	}

	if (m_register[MONTH] == 13)
	{
		m_register[MONTH] = 1;
		m_register[YEAR]++;
	}
}


//-------------------------------------------------
//  cs_w - chip select input
//-------------------------------------------------

WRITE_LINE_MEMBER( e0516_device::cs_w )
{
	m_cs = state;

	if (m_cs)
	{
		m_data_latch = 0;
		m_reg_latch = 0;
		m_bits = 0;
		m_state = STATE_ADDRESS;
	}
}


//-------------------------------------------------
//  clk_w - serial clock input
//-------------------------------------------------

WRITE_LINE_MEMBER( e0516_device::clk_w )
{
	m_clk = state;

	if (m_cs || m_clk) return;

	m_bits++;

	if (m_state == STATE_ADDRESS)
	{
		if (LOG) logerror("E05-16 '%s' Command Bit %u\n", tag(), m_dio);

		// command
		m_reg_latch |= m_dio << 3;
		m_reg_latch >>= 1;

		if (m_bits == 4)
		{
			m_state = STATE_DATA;
			m_bits = 0;

			if (BIT(m_reg_latch, 0))
			{
				// load register value to data latch
				m_data_latch = m_register[m_reg_latch >> 1];
			}
		}
	}
	else
	{
		// data
		if (BIT(m_reg_latch, 0))
		{
			// read
			if (LOG) logerror("E05-16 '%s' Data Bit OUT %u\n", tag(), m_dio);

			m_dio = BIT(m_data_latch, 0);
			m_data_latch >>= 1;
		}
		else
		{
			// write
			if (LOG) logerror("E05-16 '%s' Data Bit IN %u\n", tag(), m_dio);

			m_data_latch |= m_dio << 7;
			m_data_latch >>= 1;
		}

		if (m_bits == 8)
		{
			m_state = STATE_ADDRESS;
			m_bits = 0;

			if (!BIT(m_reg_latch, 0))
			{
				// write latched data to register
				m_register[m_reg_latch >> 1] = m_data_latch;
			}
		}
	}
}


//-------------------------------------------------
//  dio_w - serial data input
//-------------------------------------------------

WRITE_LINE_MEMBER( e0516_device::dio_w )
{
	m_dio = state;
}


//-------------------------------------------------
//  do_r - serial data output
//-------------------------------------------------

READ_LINE_MEMBER( e0516_device::dio_r )
{
	return m_dio;
}
