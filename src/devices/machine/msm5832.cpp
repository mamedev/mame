// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    OKI MSM5832 Real Time Clock/Calendar emulation

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


// device type definition
const device_type MSM5832 = &device_creator<msm5832_device>;


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



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  msm5832_device - constructor
//-------------------------------------------------

msm5832_device::msm5832_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, MSM5832, "MSM5832", tag, owner, clock, "msm5832", __FILE__),
		device_rtc_interface(mconfig, *this),
		m_hold(0),
		m_address(0),
		m_read(0),
		m_write(0),
		m_cs(0)
{
	for (auto & elem : m_reg)
		elem = 0;
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
//  device_reset - device-specific reset
//-------------------------------------------------

void msm5832_device::device_reset()
{
	set_current_time(machine());
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
//  rtc_clock_updated -
//-------------------------------------------------

void msm5832_device::rtc_clock_updated(int year, int month, int day, int day_of_week, int hour, int minute, int second)
{
	write_counter(REGISTER_Y1, year);
	write_counter(REGISTER_MO1, month);
	write_counter(REGISTER_D1, day);
	m_reg[REGISTER_W] = day_of_week-1;
	write_counter(REGISTER_H1, hour);
	write_counter(REGISTER_MI1, minute);
	write_counter(REGISTER_S1, second);
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
		else if (m_address <= REGISTER_Y10)
		{
			data = m_reg[m_address];
		}
		else
		{
			// Otrona Attache CP/M BIOS checks unused registers to detect it
			data = 0x0f;
		}
	}

	if (LOG) logerror("MSM5832 '%s' Register Read %01x: %01x\n", tag().c_str(), m_address, data & 0x0f);

	return data & 0x0f;
}


//-------------------------------------------------
//  data_w -
//-------------------------------------------------

WRITE8_MEMBER( msm5832_device::data_w )
{
	if (LOG) logerror("MSM5832 '%s' Register Write %01x: %01x\n", tag().c_str(), m_address, data & 0x0f);

	if (m_cs && m_write)
	{
		if (m_address == REGISTER_REF)
		{
			// TODO reference output
		}
		else if (m_address <= REGISTER_Y10)
		{
			m_reg[m_address] = data & 0x0f;

			set_time(false, read_counter(REGISTER_Y1), read_counter(REGISTER_MO1), read_counter(REGISTER_D1), m_reg[REGISTER_W],
				read_counter(REGISTER_H1), read_counter(REGISTER_MI1), read_counter(REGISTER_S1));
		}
	}
}


//-------------------------------------------------
//  address_w -
//-------------------------------------------------

void msm5832_device::address_w(UINT8 data)
{
	if (LOG) logerror("MSM5832 '%s' Address: %01x\n", tag().c_str(), data & 0x0f);

	m_address = data & 0x0f;
}


//-------------------------------------------------
//  adj_w -
//-------------------------------------------------

WRITE_LINE_MEMBER( msm5832_device::adj_w )
{
	if (LOG) logerror("MSM5832 '%s' 30 ADJ: %u\n", tag().c_str(), state);

	if (state)
	{
		adjust_seconds();
	}
}


//-------------------------------------------------
//  test_w -
//-------------------------------------------------

WRITE_LINE_MEMBER( msm5832_device::test_w )
{
	if (LOG) logerror("MSM5832 '%s' TEST: %u\n", tag().c_str(), state);
}


//-------------------------------------------------
//  hold_w -
//-------------------------------------------------

WRITE_LINE_MEMBER( msm5832_device::hold_w )
{
	if (LOG) logerror("MSM5832 '%s' HOLD: %u\n", tag().c_str(), state);

	m_hold = state;
}


//-------------------------------------------------
//  read_w -
//-------------------------------------------------

WRITE_LINE_MEMBER( msm5832_device::read_w )
{
	if (LOG) logerror("MSM5832 '%s' READ: %u\n", tag().c_str(), state);

	m_read = state;
}


//-------------------------------------------------
//  write_w -
//-------------------------------------------------

WRITE_LINE_MEMBER( msm5832_device::write_w )
{
	if (LOG) logerror("MSM5832 '%s' WR: %u\n", tag().c_str(), state);

	m_write = state;
}


//-------------------------------------------------
//  cs_w -
//-------------------------------------------------

WRITE_LINE_MEMBER( msm5832_device::cs_w )
{
	if (LOG) logerror("MSM5832 '%s' CS: %u\n", tag().c_str(), state);

	m_cs = state;
}
