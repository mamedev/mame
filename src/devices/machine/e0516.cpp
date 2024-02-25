// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Microelectronic-Marin E050-16 Real Time Clock emulation

**********************************************************************/

#include "emu.h"
#include "e0516.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

//#define VERBOSE 1
#include "logmacro.h"


// states
enum
{
	STATE_ADDRESS = 0,
	STATE_HI_Z,
	STATE_DATA_READ,
	STATE_DATA_WRITE
};



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

// device type definition
DEFINE_DEVICE_TYPE(E0516, e0516_device, "e0516", "E05-16 RTC")

//-------------------------------------------------
//  e0516_device - constructor
//-------------------------------------------------

e0516_device::e0516_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, E0516, tag, owner, clock),
	device_rtc_interface(mconfig, *this),
	m_read_outsel(*this, 1),
	m_write_sec(*this),
	m_write_min(*this),
	m_write_hrs(*this),
	m_write_day(*this),
	m_cs(1),
	m_clk(0),
	m_cycle(0),
	m_data_latch(0),
	m_reg_latch(0),
	m_state(STATE_ADDRESS),
	m_bits_left(4),
	m_dio(0),
	m_reset(1),
	m_timer(nullptr)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void e0516_device::device_start()
{
	// allocate timers
	m_timer = timer_alloc(FUNC(e0516_device::timer_tick), this);
	m_timer->adjust(attotime::from_hz(clock() / 32768), 0, attotime::from_hz(clock() / 32768));

	// state saving
	save_item(NAME(m_cs));
	save_item(NAME(m_clk));
	save_item(NAME(m_cycle));
	save_item(NAME(m_data_latch));
	save_item(NAME(m_reg_latch));
	save_item(NAME(m_state));
	save_item(NAME(m_bits_left));
	save_item(NAME(m_dio));
	save_item(NAME(m_reset));
}


//-------------------------------------------------
//  timer_tick - call into dirtc at 1Hz to
//  increment the seconds counter
//-------------------------------------------------

TIMER_CALLBACK_MEMBER(e0516_device::timer_tick)
{
	if (m_reset)
	{
		advance_seconds();
	}
}


//-------------------------------------------------
//  cs_w - chip select input
//-------------------------------------------------

void e0516_device::cs_w(int state)
{
	if (m_cs != state)
	{
		LOG("E05-16 '%s' CS %u\n", tag(), state);
	}

	if (!m_cs && state)
	{
		if ((m_state == STATE_DATA_WRITE) && (m_bits_left == 0) && (get_address() != 7))
		{
			LOG("E05-16 '%s' Write Register %u : %02x\n", tag(), get_address(), m_data_latch);

			// write latched data to register
			set_clock_register(get_address(), bcd_to_integer(m_data_latch));
		}

		m_cycle = 0;
		m_data_latch = 0;
		m_reg_latch = 0;
		m_bits_left = 4;
		m_state = STATE_ADDRESS;
	}

	m_cs = state;
}


//-------------------------------------------------
//  clk_w - serial clock input
//-------------------------------------------------

void e0516_device::clk_w(int state)
{
	if (m_cs) return;
	if (m_clk == state) return;
	m_clk = state;

	if (m_clk) {
		m_cycle++;
		LOG("E05-16 '%s' CLK %u\n", tag(), m_cycle);
	}

	if (!m_bits_left) return;

	if (!m_clk && (m_state == STATE_HI_Z))
	{
		m_state = STATE_DATA_READ;
		return;
	}

	if (!m_clk && (m_state != STATE_DATA_READ)) return;
	if (m_clk && (m_state == STATE_DATA_READ)) return;

	m_bits_left--;
	if ((m_state == STATE_DATA_READ) && (m_bits_left == 56) && !m_read_outsel()) return;

	if (m_state == STATE_ADDRESS)
	{
		LOG("E05-16 '%s' Command Bit %u\n", tag(), m_dio);

		// command
		m_reg_latch <<= 1;
		m_reg_latch |= m_dio;
		m_reg_latch &= 0x0f;

		if (m_bits_left == 0)
		{
			if (BIT(m_reg_latch, 0))
			{
				m_state = STATE_DATA_READ;

				if (get_address() == 7)
				{
					LOG("E05-16 '%s' Continuous Read-Out Mode\n", tag());

					// continuous read-out mode
					m_bits_left = 56;

					// load all register values to data latch
					m_data_latch = convert_to_bcd(get_clock_register(RTC_SECOND));
					m_data_latch <<= 8;
					m_data_latch = (m_data_latch & ~0xff) | convert_to_bcd(get_clock_register(RTC_DAY_OF_WEEK));
					m_data_latch <<= 8;
					m_data_latch = (m_data_latch & ~0xff) | convert_to_bcd(get_clock_register(RTC_YEAR));
					m_data_latch <<= 8;
					m_data_latch = (m_data_latch & ~0xff) | convert_to_bcd(get_clock_register(RTC_MONTH));
					m_data_latch <<= 8;
					m_data_latch = (m_data_latch & ~0xff) | convert_to_bcd(get_clock_register(RTC_DAY));
					m_data_latch <<= 8;
					m_data_latch = (m_data_latch & ~0xff) | convert_to_bcd(get_clock_register(RTC_MINUTE));
					m_data_latch <<= 8;
					m_data_latch = (m_data_latch & ~0xff) | convert_to_bcd(get_clock_register(RTC_HOUR));
				}
				else
				{
					LOG("E05-16 '%s' Read Register %u\n", tag(), get_address());

					m_bits_left = 8;

					// load register value to data latch
					m_data_latch = convert_to_bcd(get_clock_register(get_address()));
				}

				if (!m_read_outsel())
				{
					m_state = STATE_HI_Z;
				}
			}
			else
			{
				m_state = STATE_DATA_WRITE;

				if (get_address() == 7)
				{
					LOG("E05-16 '%s' Continuous Write-In Mode\n", tag());

					// continuous write-in mode
					m_bits_left = 56;
				}
				else
				{
					LOG("E05-16 '%s' Write Register %u\n", tag(), get_address());

					m_bits_left = 8;
				}
			}
		}
	}
	else
	{
		// data
		if (m_state == STATE_DATA_READ)
		{
			// read
			m_dio = BIT(m_data_latch, 0);
			m_data_latch >>= 1;

			LOG("E05-16 '%s' Data Bit OUT %u\n", tag(), m_dio);
		}
		else
		{
			// write
			LOG("E05-16 '%s' Data Bit IN %u\n", tag(), m_dio);

			m_data_latch <<= 1;
			m_data_latch |= m_dio;

			if (get_address() == 7)
			{
				switch (m_bits_left)
				{
					case 48:
						LOG("E05-16 '%s' Write Register %u : %02x\n", tag(), RTC_HOUR, m_data_latch & 0xff);
						set_clock_register(RTC_HOUR, bcd_to_integer(m_data_latch & 0xff));
						break;
					case 40:
						LOG("E05-16 '%s' Write Register %u : %02x\n", tag(), RTC_MINUTE, m_data_latch & 0xff);
						set_clock_register(RTC_MINUTE, bcd_to_integer(m_data_latch & 0xff));
						break;
					case 32:
						LOG("E05-16 '%s' Write Register %u : %02x\n", tag(), RTC_DAY, m_data_latch & 0xff);
						set_clock_register(RTC_DAY, bcd_to_integer(m_data_latch & 0xff));
						break;
					case 24:
						LOG("E05-16 '%s' Write Register %u : %02x\n", tag(), RTC_MONTH, m_data_latch & 0xff);
						set_clock_register(RTC_MONTH, bcd_to_integer(m_data_latch & 0xff));
						break;
					case 16:
						LOG("E05-16 '%s' Write Register %u : %02x\n", tag(), RTC_YEAR, m_data_latch & 0xff);
						set_clock_register(RTC_YEAR, bcd_to_integer(m_data_latch & 0xff));
						break;
					case 8:
						LOG("E05-16 '%s' Write Register %u : %02x\n", tag(), RTC_DAY_OF_WEEK, m_data_latch & 0xff);
						set_clock_register(RTC_DAY_OF_WEEK, bcd_to_integer(m_data_latch & 0xff));
						break;
					case 0:
						LOG("E05-16 '%s' Write Register %u : %02x\n", tag(), RTC_SECOND, m_data_latch & 0xff);
						set_clock_register(RTC_SECOND, bcd_to_integer(m_data_latch & 0xff));
						break;
				}
			}
		}
	}
}


//-------------------------------------------------
//  dio_w - serial data input
//-------------------------------------------------

void e0516_device::dio_w(int state)
{
	LOG("E05-16 '%s' DIO %u\n", tag(), state);

	if ((m_state != STATE_DATA_READ) && (m_state != STATE_HI_Z))
	{
		m_dio = state;
	}
}


//-------------------------------------------------
//  do_r - serial data output
//-------------------------------------------------

int e0516_device::dio_r()
{
	if (m_cs || (m_state == STATE_HI_Z))
	{
		// high impedance
		return 0;
	}

	return m_dio;
}


//-------------------------------------------------
//  reset_w - reset input
//-------------------------------------------------

void e0516_device::reset_w(int state)
{
	LOG("E05-16 '%s' RESET %u\n", tag(), state);

	if (m_reset && !state)
	{
		set_clock_register(RTC_SECOND, 0);
		set_clock_register(RTC_MINUTE, 0);
		set_clock_register(RTC_HOUR, 0);
		set_clock_register(RTC_DAY, 1);
		set_clock_register(RTC_MONTH, 1);
		set_clock_register(RTC_DAY_OF_WEEK, 1);
		set_clock_register(RTC_YEAR, 0);
	}

	m_reset = state;
}


//-------------------------------------------------
//  rtc_clock_updated -
//-------------------------------------------------

void e0516_device::rtc_clock_updated(int year, int month, int day, int day_of_week, int hour, int minute, int second)
{
}
