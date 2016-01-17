// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    OKI MSM58321RS Real Time Clock/Calendar emulation

**********************************************************************/

/*

    TODO:

    - leap year
    - test
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

static const char *reg_name(UINT8 address)
{
	switch(address)
	{
	case REGISTER_S1: return "S1";
	case REGISTER_S10: return "S10";
	case REGISTER_MI1: return "MI1";
	case REGISTER_MI10: return "MI10";
	case REGISTER_H1: return "H1";
	case REGISTER_H10: return "H10";
	case REGISTER_W: return "W";
	case REGISTER_D1: return "D1";
	case REGISTER_D10: return "D10";
	case REGISTER_MO1: return "MO1";
	case REGISTER_MO10: return "MO10";
	case REGISTER_Y1: return "Y1";
	case REGISTER_Y10: return "Y10";
	case REGISTER_RESET: return "RESET";
	case REGISTER_REF0: return "REF0";
	case REGISTER_REF1: return "REF1";
	}

	return "INVALID REGISTER";
}

enum
{
	H10_PM = 4,
	H10_24 = 8
};


//**************************************************************************
//  INLINE HELPERS
//**************************************************************************

//-------------------------------------------------
//  read_counter -
//-------------------------------------------------

inline int msm58321_device::read_counter(int counter)
{
	int data = m_reg[counter];

	if (counter == REGISTER_H1)
	{
		int h10 = m_reg[REGISTER_H10];

		if (h10 & H10_24)
		{
			data += (h10 & 3) * 10;
		}
		else
		{
			data += (h10 & 1) * 10;

			if (h10 & H10_PM)
			{
				if (data != 12)
				{
					data += 12;
				}
			}
			else if (data == 12)
			{
				data = 0;
			}
		}
	}
	else
	{
		data += (m_reg[counter + 1] * 10);
	}

	return data;
}


//-------------------------------------------------
//  write_counter -
//-------------------------------------------------

inline void msm58321_device::write_counter(int address, int data)
{
	int flag = 0;

	switch (address)
	{
	case REGISTER_H1:
		flag = m_reg[REGISTER_H10] & H10_24;
		if (!flag)
		{
			if (data >= 12)
			{
				data -= 12;
				flag = H10_PM;
			}

			if (data == 0)
			{
				data = 12;
			}
		}
		break;

	case REGISTER_D1:
		flag = (m_reg[REGISTER_D10] & ~3);
		break;
	}

	m_reg[address] = data % 10;
	m_reg[address + 1] = (data / 10) | flag;
}



//-------------------------------------------------
//  msm58321_device - constructor
//-------------------------------------------------

msm58321_device::msm58321_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, MSM58321, "MSM58321", tag, owner, clock, "msm58321", __FILE__),
	device_rtc_interface(mconfig, *this),
	device_nvram_interface(mconfig, *this),
	m_year0(0),
	m_default_24h(false),
	m_d0_handler(*this),
	m_d1_handler(*this),
	m_d2_handler(*this),
	m_d3_handler(*this),
	m_busy_handler(*this),
	m_cs2(0),
	m_write(0),
	m_read(0),
	m_d0_in(0),
	m_d0_out(0),
	m_d1_in(0),
	m_d1_out(0),
	m_d2_in(0),
	m_d2_out(0),
	m_d3_in(0),
	m_d3_out(0),
	m_address_write(0),
	m_busy(0),
	m_stop(0),
	m_test(0),
	m_cs1(0),
	m_address(0xf)
{
	memset(m_reg, 0x00, sizeof(m_reg));
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void msm58321_device::device_start()
{
	// resolve callbacks
	m_d0_handler.resolve_safe();
	m_d1_handler.resolve_safe();
	m_d2_handler.resolve_safe();
	m_d3_handler.resolve_safe();
	m_busy_handler.resolve_safe();

	// allocate timers
	m_clock_timer = timer_alloc(TIMER_CLOCK);
	m_clock_timer->adjust(attotime::from_hz(clock() / 32768), 0, attotime::from_hz(clock() / 32768));

	m_busy_timer = timer_alloc(TIMER_BUSY);
	m_busy_timer->adjust(attotime::from_hz(clock() / 16384), 0, attotime::from_hz(clock() / 16384));

	// state saving
	save_item(NAME(m_cs2));
	save_item(NAME(m_write));
	save_item(NAME(m_read));
	save_item(NAME(m_d0_in));
	save_item(NAME(m_d0_out));
	save_item(NAME(m_d1_in));
	save_item(NAME(m_d1_out));
	save_item(NAME(m_d2_in));
	save_item(NAME(m_d2_out));
	save_item(NAME(m_d3_in));
	save_item(NAME(m_d3_out));
	save_item(NAME(m_address_write));
	save_item(NAME(m_busy));
	save_item(NAME(m_stop));
	save_item(NAME(m_test));
	save_item(NAME(m_cs1));
	save_item(NAME(m_address));
	save_item(NAME(m_reg));

	set_current_time(machine());

	update_output();
}


//-------------------------------------------------
//  device_timer - handler timer events
//-------------------------------------------------

void msm58321_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch (id)
	{
	case TIMER_CLOCK:
		if (!m_stop)
			advance_seconds();
		break;

	case TIMER_BUSY:
		if (!m_cs1 || !m_cs2 || !m_write || m_address != REGISTER_RESET)
		{
			m_busy = !m_busy;
			m_busy_handler(m_busy);
		}
		break;
	}
}


//-------------------------------------------------
//  rtc_clock_updated -
//-------------------------------------------------

void msm58321_device::rtc_clock_updated(int year, int month, int day, int day_of_week, int hour, int minute, int second)
{
	write_counter(REGISTER_Y1, (year - m_year0) % 100);
	write_counter(REGISTER_MO1, month);
	write_counter(REGISTER_D1, day);
	m_reg[REGISTER_W] = day_of_week;
	write_counter(REGISTER_H1, hour);
	write_counter(REGISTER_MI1, minute);
	write_counter(REGISTER_S1, second);

	update_output();
}

//-------------------------------------------------
//  nvram_default - called to initialize NVRAM to
//  its default state
//-------------------------------------------------

void msm58321_device::nvram_default()
{
	for (auto & elem : m_reg)
		elem = 0;

	if (m_default_24h)
		m_reg[REGISTER_H10] = H10_24;

	clock_updated();
}


//-------------------------------------------------
//  nvram_read - called to read NVRAM from the
//  .nv file
//-------------------------------------------------

void msm58321_device::nvram_read(emu_file &file)
{
	file.read(m_reg, sizeof(m_reg));

	clock_updated();
}


//-------------------------------------------------
//  nvram_write - called to write NVRAM to the
//  .nv file
//-------------------------------------------------

void msm58321_device::nvram_write(emu_file &file)
{
	file.write(m_reg, sizeof(m_reg));
}

//-------------------------------------------------
//  update_output -
//-------------------------------------------------

void msm58321_device::update_output()
{
	UINT8 data = 0xf;

	if (m_cs1 && m_cs2 && m_read)
	{
		switch (m_address)
		{
		case REGISTER_RESET:
			data = 0;
			break;

		case REGISTER_REF0:
		case REGISTER_REF1:
			// TODO: output reference values
			data = 0;
			break;

		default:
			data = m_reg[m_address];
			break;
		}

		if (LOG) logerror("MSM58321 '%s' Register Read %s (%01x): %01x\n", tag().c_str(), reg_name(m_address), m_address, data & 0x0f);
	}

	int d0 = (data >> 0) & 1;
	if (m_d0_out != d0)
	{
		m_d0_out = d0;
		m_d0_handler(d0);
	}

	int d1 = (data >> 1) & 1;
	if (m_d1_out != d1)
	{
		m_d1_out = d1;
		m_d1_handler(d1);
	}

	int d2 = (data >> 2) & 1;
	if (m_d2_out != d2)
	{
		m_d2_out = d2;
		m_d2_handler(d2);
	}

	int d3 = (data >> 3) & 1;
	if (m_d3_out != d3)
	{
		m_d3_out = d3;
		m_d3_handler(d3);
	}
}


//-------------------------------------------------
//  update_input() -
//-------------------------------------------------

void msm58321_device::update_input()
{
	if (m_cs1 && m_cs2)
	{
		UINT8 data = m_d0_in | (m_d1_in << 1) | (m_d2_in << 2) | (m_d3_in << 3);

		if (m_address_write)
		{
			if (LOG) logerror("MSM58321 '%s' Latch Address %01x\n", tag().c_str(), data);

			// latch address
			m_address = data;
		}

		if (m_write)
		{
			switch(m_address)
			{
			case REGISTER_RESET:
				if (LOG) logerror("MSM58321 '%s' Reset\n", tag().c_str());

				if (!m_busy)
				{
					m_busy = 1;
					m_busy_handler(m_busy);
				}
				break;

			case REGISTER_REF0:
			case REGISTER_REF1:
				if (LOG) logerror("MSM58321 '%s' Reference Signal\n", tag().c_str());
				break;

			default:
				if (LOG) logerror("MSM58321 '%s' Register Write %s (%01x): %01x\n", tag().c_str(), reg_name(m_address), m_address, data);

				switch (m_address)
				{
				case REGISTER_S10:
				case REGISTER_MI10:
				case REGISTER_W:
					m_reg[m_address] = data & 7;
					break;

				case REGISTER_H10:
					if (data & H10_24)
					{
						// "When D3 = 1 is written, the D2 bit is reset inside the IC."
						// but it doesn't say if this is done immediately or on the next update
						m_reg[m_address] = data & ~H10_PM;
					}
					else
					{
						m_reg[m_address] = data;
					}
					break;

				case REGISTER_MO10:
					m_reg[m_address] = data & 1;
					break;

				default:
					m_reg[m_address] = data;
					break;
				}

				set_time(false, read_counter(REGISTER_Y1) + m_year0, read_counter(REGISTER_MO1), read_counter(REGISTER_D1), m_reg[REGISTER_W],
					read_counter(REGISTER_H1), read_counter(REGISTER_MI1), read_counter(REGISTER_S1));
				break;
			}
		}
	}
}

//-------------------------------------------------
//  cs2_w -
//-------------------------------------------------

WRITE_LINE_MEMBER( msm58321_device::cs2_w )
{
	if (m_cs2 != state)
	{
		if (LOG) logerror("MSM58321 '%s' CS2: %u\n", tag().c_str(), state);

		m_cs2 = state;

		update_input();
	}
}


//-------------------------------------------------
//  write_w -
//-------------------------------------------------

WRITE_LINE_MEMBER( msm58321_device::write_w )
{
	if (m_write != state)
	{
		if (LOG) logerror("MSM58321 '%s' WRITE: %u\n", tag().c_str(), state);

		m_write = state;

		update_input();
	}
}


//-------------------------------------------------
//  read_w -
//-------------------------------------------------

WRITE_LINE_MEMBER( msm58321_device::read_w )
{
	if (m_read != state)
	{
		if (LOG) logerror("MSM58321 '%s' READ: %u\n", tag().c_str(), state);

		m_read = state;

		update_output();
	}
}



//-------------------------------------------------
//  d0_w -
//-------------------------------------------------

WRITE_LINE_MEMBER( msm58321_device::d0_w )
{
	if (m_d0_in != state)
	{
		m_d0_in = state;

		update_input();
	}
}


//-------------------------------------------------
//  d1_w -
//-------------------------------------------------

WRITE_LINE_MEMBER( msm58321_device::d1_w )
{
	if (m_d1_in != state)
	{
		m_d1_in = state;

		update_input();
	}
}


//-------------------------------------------------
//  d2_w -
//-------------------------------------------------

WRITE_LINE_MEMBER( msm58321_device::d2_w )
{
	if (m_d2_in != state)
	{
		m_d2_in = state;

		update_input();
	}
}


//-------------------------------------------------
//  d3_w -
//-------------------------------------------------

WRITE_LINE_MEMBER( msm58321_device::d3_w )
{
	if (m_d3_in != state)
	{
		m_d3_in = state;

		update_input();
	}
}


//-------------------------------------------------
//  address_write_w -
//-------------------------------------------------

WRITE_LINE_MEMBER( msm58321_device::address_write_w )
{
	if (m_address_write != state)
	{
		if (LOG) logerror("MSM58321 '%s' ADDRESS WRITE: %u\n", tag().c_str(), state);

		m_address_write = state;

		update_input();
	}
}


//-------------------------------------------------
//  stop_w -
//-------------------------------------------------

WRITE_LINE_MEMBER( msm58321_device::stop_w )
{
	if (m_stop != state)
	{
		if (LOG) logerror("MSM58321 '%s' STOP: %u\n", tag().c_str(), state);

		m_stop = state;
	}
}


//-------------------------------------------------
//  test_w -
//-------------------------------------------------

WRITE_LINE_MEMBER( msm58321_device::test_w )
{
	if (m_test != state)
	{
		if (LOG) logerror("MSM58321 '%s' TEST: %u\n", tag().c_str(), state);

		m_test = state;
	}
}



//-------------------------------------------------
//  cs1_w -
//-------------------------------------------------

WRITE_LINE_MEMBER( msm58321_device::cs1_w )
{
	if (m_cs1 != state)
	{
		if (LOG) logerror("MSM58321 '%s' CS1: %u\n", tag().c_str(), state);

		m_cs1 = state;

		update_input();
	}
}
