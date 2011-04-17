/**********************************************************************

    OKI MSM5832 Real Time Clock/Calendar emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************/

/*

    TODO:

    - 12/24 hour
    - AM/PM
    - leap year
    - test input
    - reference signal output

*/

#include "msm5832.h"
#include "machine/devhelpr.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define LOG 0


// registers
enum
{
	REGISTER_S1 = 0,
	REGISTER_S10,
	REGISTER_MI1,
	REGISTER_MI10,
	REGISTER_H1,
	REGISTER_H10,
	REGISTER_W,
	REGISTER_D1,
	REGISTER_D10,
	REGISTER_MO1,
	REGISTER_MO10,
	REGISTER_Y1,
	REGISTER_Y10,
	REGISTER_REF = 15
};


// days per month
static const int DAYS_PER_MONTH[12] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// devices
const device_type MSM5832 = msm5832_device_config::static_alloc_device_config;



//**************************************************************************
//  DEVICE CONFIGURATION
//**************************************************************************

GENERIC_DEVICE_CONFIG_SETUP(msm5832, "MSM5832")



//**************************************************************************
//  INLINE HELPERS
//**************************************************************************

//-------------------------------------------------
//  read_counter -
//-------------------------------------------------

inline int msm5832_device::read_counter(int counter)
{
	return (m_reg[counter + 1] * 10) + m_reg[counter];
}


//-------------------------------------------------
//  write_counter -
//-------------------------------------------------

inline void msm5832_device::write_counter(int counter, int value)
{
	m_reg[counter] = value % 10;
	m_reg[counter + 1] = value / 10;
}


//-------------------------------------------------
//  advance_seconds -
//-------------------------------------------------

inline void msm5832_device::advance_seconds()
{
	int seconds = read_counter(REGISTER_S1);

	seconds++;

	if (seconds > 59)
	{
		seconds = 0;

		advance_minutes();
	}

	write_counter(REGISTER_S1, seconds);
}


//-------------------------------------------------
//  advance_minutes -
//-------------------------------------------------

inline void msm5832_device::advance_minutes()
{
	int minutes = read_counter(REGISTER_MI1);
	int hours = read_counter(REGISTER_H1);
	int days = read_counter(REGISTER_D1);
	int month = read_counter(REGISTER_MO1);
	int year = read_counter(REGISTER_Y1);
	int day_of_week = m_reg[REGISTER_W];

	minutes++;

	if (minutes > 59)
	{
		minutes = 0;
		hours++;
	}

	if (hours > 23)
	{
		hours = 0;
		days++;
		day_of_week++;
	}

	if (day_of_week > 6)
	{
		day_of_week++;
	}

	if (days > DAYS_PER_MONTH[month - 1])
	{
		days = 1;
		month++;
	}

	if (month > 12)
	{
		month = 1;
		year++;
	}

	if (year > 99)
	{
		year = 0;
	}

	write_counter(REGISTER_MI1, minutes);
	write_counter(REGISTER_H1, hours);
	write_counter(REGISTER_D1, days);
	write_counter(REGISTER_MO1, month);
	write_counter(REGISTER_Y1, year);
	m_reg[REGISTER_W] = day_of_week;
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  msm5832_device - constructor
//-------------------------------------------------

msm5832_device::msm5832_device(running_machine &_machine, const msm5832_device_config &config)
    : device_t(_machine, config),
	  m_hold(0),
	  m_address(0),
	  m_read(0),
	  m_write(0),
	  m_cs(0),
      m_config(config)
{
	for (int i = 0; i < 13; i++)
		m_reg[i] = 0;
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void msm5832_device::device_start()
{
	// allocate timers
	m_clock_timer = timer_alloc(TIMER_CLOCK);
	m_clock_timer->adjust(attotime::from_hz(clock() / 32768), 0, attotime::from_hz(clock() / 32768));

	// state saving
	save_item(NAME(m_reg));
	save_item(NAME(m_hold));
	save_item(NAME(m_address));
	save_item(NAME(m_read));
	save_item(NAME(m_write));
	save_item(NAME(m_cs));
}


//-------------------------------------------------
//  device_timer - handler timer events
//-------------------------------------------------

void msm5832_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch (id)
	{
	case TIMER_CLOCK:
		if (!m_hold)
		{
			advance_seconds();
		}
		break;
	}
}


//-------------------------------------------------
//  data_r -
//-------------------------------------------------

READ8_MEMBER( msm5832_device::data_r )
{
	UINT8 data = 0;

	if (m_cs && m_read)
	{
		if (m_address == REGISTER_REF)
		{
			// TODO reference output
		}
		else
		{
			data = m_reg[m_address];
		}
	}

	if (LOG) logerror("MSM5832 '%s' Register Read %01x: %01x\n", tag(), m_address, data & 0x0f);

	return data & 0x0f;
}


//-------------------------------------------------
//  data_w -
//-------------------------------------------------

WRITE8_MEMBER( msm5832_device::data_w )
{
	if (LOG) logerror("MSM5832 '%s' Register Write %01x: %01x\n", tag(), m_address, data & 0x0f);

	if (m_cs && m_write)
	{
		m_reg[m_address] = data & 0x0f;
	}
}


//-------------------------------------------------
//  address_w -
//-------------------------------------------------

void msm5832_device::address_w(UINT8 data)
{
	if (LOG) logerror("MSM5832 '%s' Address: %01x\n", tag(), data & 0x0f);

	m_address = data & 0x0f;
}


//-------------------------------------------------
//  adj_w -
//-------------------------------------------------

WRITE_LINE_MEMBER( msm5832_device::adj_w )
{
	if (LOG) logerror("MSM5832 '%s' 30 ADJ: %u\n", tag(), state);

	if (state)
	{
		int seconds = read_counter(REGISTER_S1);

		if (seconds < 30)
		{
			write_counter(REGISTER_S1, 0);
		}
		else
		{
			write_counter(REGISTER_S1, 0);
			advance_minutes();
		}
	}
}


//-------------------------------------------------
//  test_w -
//-------------------------------------------------

WRITE_LINE_MEMBER( msm5832_device::test_w )
{
	if (LOG) logerror("MSM5832 '%s' TEST: %u\n", tag(), state);
}


//-------------------------------------------------
//  hold_w -
//-------------------------------------------------

WRITE_LINE_MEMBER( msm5832_device::hold_w )
{
	if (LOG) logerror("MSM5832 '%s' HOLD: %u\n", tag(), state);

	m_hold = state;
}


//-------------------------------------------------
//  read_w -
//-------------------------------------------------

WRITE_LINE_MEMBER( msm5832_device::read_w )
{
	if (LOG) logerror("MSM5832 '%s' READ: %u\n", tag(), state);

	m_read = state;
}


//-------------------------------------------------
//  write_w -
//-------------------------------------------------

WRITE_LINE_MEMBER( msm5832_device::write_w )
{
	if (LOG) logerror("MSM5832 '%s' WR: %u\n", tag(), state);

	m_write = state;
}


//-------------------------------------------------
//  cs_w -
//-------------------------------------------------

WRITE_LINE_MEMBER( msm5832_device::cs_w )
{
	if (LOG) logerror("MSM5832 '%s' CS: %u\n", tag(), state);

	m_cs = state;
}
