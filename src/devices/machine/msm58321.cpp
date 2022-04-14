// license:BSD-3-Clause
// copyright-holders:Curt Coder,Sven Schnelle
/**********************************************************************

    OKI MSM58321RS Real Time Clock/Calendar emulation

**********************************************************************/

/*

    TODO:

    - non gregorian leap year

*/

#include "emu.h"
#include "msm58321.h"

//#define VERBOSE 1
#include "logmacro.h"



// device type definition
DEFINE_DEVICE_TYPE(MSM58321, msm58321_device, "msm58321", "OKI MSM58321 RTC")


//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

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

static const char *reg_name(uint8_t address)
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
		else if (h10 & H10_PM)
		{
			data += 12 + (h10 & 1) * 10;
		}
		else
		{
			data += (h10 & 1) * 10;
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

			if ((m_reg[REGISTER_H10] & H10_PM) && data == 0)
				data = 12;
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

msm58321_device::msm58321_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, MSM58321, tag, owner, clock),
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
	m_address(0xf),
	m_reg{},
	m_khz_ctr(0)
{
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
	m_clock_timer->adjust(clocks_to_attotime(32768/1024), 0, clocks_to_attotime(32768/1024));

	// busy signal active period is approximately 427 µs
	m_busy_timer = timer_alloc(TIMER_BUSY);
	m_busy_timer->adjust(clocks_to_attotime(32768 - 14), 0, clocks_to_attotime(32768));

	// standard signal active period is approximately 122 µs
	m_standard_timer = timer_alloc(TIMER_STANDARD);
	m_standard_timer->adjust(clocks_to_attotime(32768-4), 0, clocks_to_attotime(32768));

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
	save_item(NAME(m_khz_ctr));
}


//-------------------------------------------------
//  device_timer - handler timer events
//-------------------------------------------------

void msm58321_device::device_timer(emu_timer &timer, device_timer_id id, int param)
{
	switch (id)
	{
	case TIMER_CLOCK:

		if (m_khz_ctr & 1)
		{
			m_reg[REGISTER_REF0] |= 1;
			m_reg[REGISTER_REF1] |= 1;
		}
		else
		{
			m_reg[REGISTER_REF0] &= ~1;
			m_reg[REGISTER_REF1] &= ~1;
		}

		if (++m_khz_ctr >= 1024)
		{
			m_khz_ctr = 0;
			if (!m_stop)
			{
				advance_seconds();
			}

			if (!m_busy)
			{
				m_busy = 1;
				m_busy_handler(m_busy);
			}
		}
		break;

	case TIMER_BUSY:
		if (!m_cs1 || !m_cs2 || !m_write || m_address != REGISTER_RESET)
		{
			m_busy = 0;
			m_busy_handler(m_busy);
		}
		break;
	case TIMER_STANDARD:
		m_reg[REGISTER_REF0] = 0x0e;
		m_reg[REGISTER_REF1] = 0x0e;
		break;
	}
}

void msm58321_device::update_standard()
{
	uint8_t reg = 0;

	if (m_reg[REGISTER_S1] == 0)
		reg |= 1 << 1;

	if (m_reg[REGISTER_MI1] == 0)
		reg |= 1 << 2;

	if (m_reg[REGISTER_H1] == 0)
		reg |= 1 << 3;

	m_reg[REGISTER_REF0] = (reg ^ 0x0e) | (m_khz_ctr & 1);
	m_reg[REGISTER_REF1] = m_reg[REGISTER_REF0];
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
	update_standard();
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

bool msm58321_device::nvram_read(util::read_stream &file)
{
	size_t actual;
	if (file.read(m_reg.data(), m_reg.size(), actual) || actual != m_reg.size())
		return false;

	clock_updated();
	return true;
}


//-------------------------------------------------
//  nvram_write - called to write NVRAM to the
//  .nv file
//-------------------------------------------------

bool msm58321_device::nvram_write(util::write_stream &file)
{
	size_t actual;
	return !file.write(m_reg.data(), m_reg.size(), actual) && actual == m_reg.size();
}

//-------------------------------------------------
//  update_output -
//-------------------------------------------------

void msm58321_device::update_output()
{
	uint8_t data = 0xf;

	if (m_cs1 && m_cs2 && m_read)
	{
		switch (m_address)
		{
		case REGISTER_RESET:
			data = 0;
			break;
		case REGISTER_W:
			data = m_reg[m_address] - 1;
			break;
		default:
			data = m_reg[m_address];
			break;
		}

		LOG("MSM58321 Register Read %s (%01x): %01x\n", reg_name(m_address), m_address, data & 0x0f);
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
		uint8_t data = m_d0_in | (m_d1_in << 1) | (m_d2_in << 2) | (m_d3_in << 3);

		if (m_address_write)
		{
			LOG("MSM58321 Latch Address %01x\n", data);

			// latch address
			m_address = data;
		}

		if (m_write)
		{
			switch(m_address)
			{
			case REGISTER_RESET:
				LOG("MSM58321 Reset\n");

				if (!m_busy)
				{
					m_busy = 1;
					m_busy_handler(m_busy);
				}
				break;

			case REGISTER_REF0:
			case REGISTER_REF1:
				LOG("MSM58321 Reference Signal\n");
				break;

			default:
				LOG("MSM58321 Register Write %s (%01x): %01x\n", reg_name(m_address), m_address, data);
				m_khz_ctr = 0;
				switch (m_address)
				{
				case REGISTER_S10:
				case REGISTER_MI10:

					m_reg[m_address] = data & 7;
					break;
				case REGISTER_W:
					m_reg[m_address] = (data & 7) + 1;
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
		LOG("MSM58321 CS2: %u\n", state);

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
		LOG("MSM58321 WRITE: %u\n", state);

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
		LOG("MSM58321 READ: %u\n", state);

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
		LOG("MSM58321 ADDRESS WRITE: %u\n", state);

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
		LOG("MSM58321 STOP: %u\n", state);

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
		LOG("MSM58321 TEST: %u\n", state);

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
		LOG("MSM58321 CS1: %u\n", state);

		m_cs1 = state;

		update_input();
	}
}
