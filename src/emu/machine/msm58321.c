/**********************************************************************

    OKI MSM58321RS Real Time Clock/Calendar emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************/

/*

    TODO:

    - 12/24 hour
    - AM/PM
    - leap year
    - stop
    - reset
    - reference registers

*/

#include "msm58321.h"


// device type definition
const device_type MSM58321 = &device_creator<msm58321_device>;


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
	REGISTER_RESET,
	REGISTER_REF0,
	REGISTER_REF1
};



//**************************************************************************
//  INLINE HELPERS
//**************************************************************************

//-------------------------------------------------
//  read_counter -
//-------------------------------------------------

inline int msm58321_device::read_counter(int counter)
{
	return (m_reg[counter + 1] * 10) + m_reg[counter];
}


//-------------------------------------------------
//  write_counter -
//-------------------------------------------------

inline void msm58321_device::write_counter(int counter, int value)
{
	m_reg[counter] = value % 10;
	m_reg[counter + 1] = value / 10;
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  msm58321_device - constructor
//-------------------------------------------------

msm58321_device::msm58321_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, MSM58321, "MSM58321", tag, owner, clock),
		device_rtc_interface(mconfig, *this)
{
	for (int i = 0; i < 13; i++)
		m_reg[i] = 0;
}


//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void msm58321_device::device_config_complete()
{
	// inherit a copy of the static data
	const msm58321_interface *intf = reinterpret_cast<const msm58321_interface *>(static_config());
	if (intf != NULL)
		*static_cast<msm58321_interface *>(this) = *intf;

	// or initialize to defaults if none provided
	else
	{
		memset(&m_out_busy_cb, 0, sizeof(m_out_busy_cb));
	}
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void msm58321_device::device_start()
{
	// resolve callbacks
	m_out_busy_func.resolve(m_out_busy_cb, *this);

	// allocate timers
	m_clock_timer = timer_alloc(TIMER_CLOCK);
	m_clock_timer->adjust(attotime::from_hz(clock() / 32768), 0, attotime::from_hz(clock() / 32768));

	m_busy_timer = timer_alloc(TIMER_BUSY);
	m_busy_timer->adjust(attotime::from_hz(clock() / 16384), 0, attotime::from_hz(clock() / 16384));

	// state saving
	save_item(NAME(m_cs1));
	save_item(NAME(m_cs2));
	save_item(NAME(m_busy));
	save_item(NAME(m_read));
	save_item(NAME(m_write));
	save_item(NAME(m_address_write));
	save_item(NAME(m_reg));
	save_item(NAME(m_latch));
	save_item(NAME(m_address));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void msm58321_device::device_reset()
{
	set_current_time(machine());
}


//-------------------------------------------------
//  device_timer - handler timer events
//-------------------------------------------------

void msm58321_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch (id)
	{
	case TIMER_CLOCK:
		advance_seconds();
		break;

	case TIMER_BUSY:
		m_out_busy_func(m_busy);
		m_busy = !m_busy;
		break;
	}
}


//-------------------------------------------------
//  rtc_clock_updated -
//-------------------------------------------------

void msm58321_device::rtc_clock_updated(int year, int month, int day, int day_of_week, int hour, int minute, int second)
{
	write_counter(REGISTER_Y1, year);
	write_counter(REGISTER_MO1, month);
	write_counter(REGISTER_D1, day);
	m_reg[REGISTER_W] = day_of_week;
	write_counter(REGISTER_H1, hour);
	write_counter(REGISTER_MI1, minute);
	write_counter(REGISTER_S1, second);
}


//-------------------------------------------------
//  read -
//-------------------------------------------------

READ8_MEMBER( msm58321_device::read )
{
	UINT8 data = 0;

	if (m_cs1 && m_cs2)
	{
		if (m_read)
		{
			switch (m_address)
			{
			case REGISTER_RESET:
				break;

			case REGISTER_REF0:
			case REGISTER_REF1:
				break;

			default:
				data = m_reg[m_address];
				break;
			}
		}

		if (m_write)
		{
			if (m_address >= REGISTER_REF0)
			{
				// TODO: output reference values
			}
		}
	}

	if (LOG) logerror("MSM58321 '%s' Register Read %01x: %01x\n", tag(), m_address, data & 0x0f);

	return data;
}


//-------------------------------------------------
//  write -
//-------------------------------------------------

WRITE8_MEMBER( msm58321_device::write )
{
	// latch data for future use
	m_latch = data & 0x0f;

	if (!m_cs1 || !m_cs2) return;

	if (m_address_write)
	{
		if (LOG) logerror("MSM58321 '%s' Latch Address %01x\n", tag(), m_latch);

		// latch address
		m_address = m_latch;
	}

	if (m_write)
	{
		switch(m_address)
		{
		case REGISTER_RESET:
			if (LOG) logerror("MSM58321 '%s' Reset\n", tag());
			break;

		case REGISTER_REF0:
		case REGISTER_REF1:
			if (LOG) logerror("MSM58321 '%s' Reference Signal\n", tag());
			break;

		default:
			if (LOG) logerror("MSM58321 '%s' Register Write %01x: %01x\n", tag(), m_address, data & 0x0f);
			m_reg[m_address] = m_latch & 0x0f;

			set_time(false, read_counter(REGISTER_Y1), read_counter(REGISTER_MO1), read_counter(REGISTER_D1), m_reg[REGISTER_W],
				read_counter(REGISTER_H1), read_counter(REGISTER_MI1), read_counter(REGISTER_S1));
			break;
		}
	}
}


//-------------------------------------------------
//  cs1_w -
//-------------------------------------------------

WRITE_LINE_MEMBER( msm58321_device::cs1_w )
{
	if (LOG) logerror("MSM58321 '%s' CS1: %u\n", tag(), state);

	m_cs1 = state;
}


//-------------------------------------------------
//  cs2_w -
//-------------------------------------------------

WRITE_LINE_MEMBER( msm58321_device::cs2_w )
{
	if (LOG) logerror("MSM58321 '%s' CS2: %u\n", tag(), state);

	m_cs2 = state;
}


//-------------------------------------------------
//  read_w -
//-------------------------------------------------

WRITE_LINE_MEMBER( msm58321_device::read_w )
{
	if (LOG) logerror("MSM58321 '%s' READ: %u\n", tag(), state);

	m_read = state;
}


//-------------------------------------------------
//  write_w -
//-------------------------------------------------

WRITE_LINE_MEMBER( msm58321_device::write_w )
{
	if (LOG) logerror("MSM58321 '%s' WRITE: %u\n", tag(), state);

	m_write = state;
}


//-------------------------------------------------
//  address_write_w -
//-------------------------------------------------

WRITE_LINE_MEMBER( msm58321_device::address_write_w )
{
	if (LOG) logerror("MSM58321 '%s' ADDRESS WRITE: %u\n", tag(), state);

	m_address_write = state;

	if (m_address_write)
	{
		if (LOG) logerror("MSM58321 '%s' Latch Address %01x\n", tag(), m_latch);

		// latch address
		m_address = m_latch;
	}
}


//-------------------------------------------------
//  stop_w -
//-------------------------------------------------

WRITE_LINE_MEMBER( msm58321_device::stop_w )
{
	if (LOG) logerror("MSM58321 '%s' STOP: %u\n", tag(), state);
}


//-------------------------------------------------
//  test_w -
//-------------------------------------------------

WRITE_LINE_MEMBER( msm58321_device::test_w )
{
	if (LOG) logerror("MSM58321 '%s' TEST: %u\n", tag(), state);
}


//-------------------------------------------------
//  busy_r -
//-------------------------------------------------

READ_LINE_MEMBER( msm58321_device::busy_r )
{
	return m_busy;
}
