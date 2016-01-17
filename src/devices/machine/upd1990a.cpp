// license:BSD-3-Clause
// copyright-holders:Curt Coder, hap
/**********************************************************************

    NEC uPD1990AC Serial I/O Calendar & Clock emulation

**********************************************************************/

/*

    TODO:
    - test mode is mostly untested
    - how does timer-interval differ from timer-pulse?

*/

#include "upd1990a.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define LOG 0



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

// device type definition
const device_type UPD1990A = &device_creator<upd1990a_device>;
const device_type UPD4990A = &device_creator<upd4990a_device>;



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  upd1990a_device - constructor
//-------------------------------------------------

upd1990a_device::upd1990a_device(const machine_config &mconfig, device_type type, std::string name, std::string tag, device_t *owner, UINT32 clock, UINT32 variant, std::string shortname, std::string source)
	: device_t(mconfig, type, name, tag, owner, clock, shortname, source),
		device_rtc_interface(mconfig, *this),
		m_write_data(*this),
		m_write_tp(*this),
		m_variant(variant)
{
}

upd1990a_device::upd1990a_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, UPD1990A, "uPD1990A", tag, owner, clock, "upd1990a", __FILE__),
		device_rtc_interface(mconfig, *this),
		m_write_data(*this),
		m_write_tp(*this),
		m_variant(TYPE_1990A)
{
}

upd4990a_device::upd4990a_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
	: upd1990a_device(mconfig, UPD4990A, "uPD4990A RTC", tag, owner, clock, TYPE_4990A, "upd4990a", __FILE__) { }


bool upd1990a_device::is_serial_mode()
{
	// uPD4990A is in serial mode if c0/1/2 = high/VDD
	return (m_variant == TYPE_4990A && m_c_unlatched == 7);
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void upd1990a_device::device_start()
{
	(void)m_variant;
	// resolve callbacks
	m_write_data.resolve_safe();
	m_write_tp.resolve_safe();

	// initialize
	set_current_time(machine());

	for (auto & elem : m_shift_reg)
		elem = 0;

	m_oe = 0;
	m_cs = 0;
	m_stb = 0;
	m_data_in = 0;
	m_data_out = 0;
	m_c = 0;
	m_clk = 0;
	m_tp = 0;
	m_c_unlatched = 0;
	m_testmode = false;

	// allocate timers
	m_timer_clock = timer_alloc(TIMER_CLOCK);
	m_timer_clock->adjust(attotime::from_hz(clock() / 32768.0), 0, attotime::from_hz(clock() / 32768.0)); // 1 second on XTAL_32_768kHz
	m_timer_tp = timer_alloc(TIMER_TP);
	m_timer_data_out = timer_alloc(TIMER_DATA_OUT);
	m_timer_test_mode = timer_alloc(TIMER_TEST_MODE);

	// state saving
	save_item(NAME(m_time_counter));
	save_item(NAME(m_shift_reg));
	save_item(NAME(m_oe));
	save_item(NAME(m_cs));
	save_item(NAME(m_stb));
	save_item(NAME(m_data_in));
	save_item(NAME(m_data_out));
	save_item(NAME(m_c));
	save_item(NAME(m_clk));
	save_item(NAME(m_tp));
	save_item(NAME(m_c_unlatched));
	save_item(NAME(m_testmode));
}


//-------------------------------------------------
//  rtc_clock_updated -
//-------------------------------------------------

void upd1990a_device::rtc_clock_updated(int year, int month, int day, int day_of_week, int hour, int minute, int second)
{
	m_time_counter[0] = convert_to_bcd(second);
	m_time_counter[1] = convert_to_bcd(minute);
	m_time_counter[2] = convert_to_bcd(hour);
	m_time_counter[3] = convert_to_bcd(day);
	m_time_counter[4] = (month << 4) | (day_of_week - 1);
	m_time_counter[5] = convert_to_bcd(year);
}


//-------------------------------------------------
//  device_timer - handler timer events
//-------------------------------------------------

void upd1990a_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch (id)
	{
	case TIMER_CLOCK:
		advance_seconds();
		break;

	case TIMER_TP:
		m_tp = !m_tp;
		m_write_tp(m_tp);
		break;

	case TIMER_DATA_OUT:
		m_data_out = !m_data_out;
		m_write_data(get_data_out());
		break;

	case TIMER_TEST_MODE:
		if (m_oe)
		{
			/* TODO: completely untested */
			/* time counter is advanced from "Second" counter input */
			int max_shift = is_serial_mode() ? 6 : 5;
			m_data_out = (m_time_counter[max_shift - 1] == 0);
			m_write_data(get_data_out());

			for (int i = 0; i < max_shift; i++)
			{
				m_time_counter[i]++;
				if (m_time_counter[i] != 0)
					return;
			}
		}
		else
		{
			/* each counter is advanced in parallel, overflow carry does not affect next counter */
			m_data_out = 0;

			int max_shift = is_serial_mode() ? 6 : 5;
			for (int i = 0; i < max_shift; i++)
			{
				m_time_counter[i]++;
				m_data_out |= (m_time_counter[i] == 0);
			}
			m_write_data(get_data_out());
		}

		break;
	}
}


//-------------------------------------------------
//  stb_w -
//-------------------------------------------------

WRITE_LINE_MEMBER( upd1990a_device::stb_w )
{
	if (!m_cs)
		return;

	if (LOG) logerror("uPD1990A '%s' STB %u\n", tag().c_str(), state);

	// rising edge
	if (!m_stb && state)
	{
		// read command
		if (is_serial_mode())
			m_c = m_shift_reg[6];
		else
		{
			m_c = m_c_unlatched;
			if (m_c == 7)
				m_c = MODE_TEST;
		}

		if (LOG) logerror("uPD1990A '%s' Command %x\n", tag().c_str(), m_c);

		// common functions
		if (m_c == MODE_REGISTER_HOLD || (m_c >= MODE_TP_64HZ && m_c < MODE_TEST))
		{
			// enable time counter
			m_timer_clock->enable(1);

			// disable testmode
			m_testmode = false;
			m_timer_test_mode->enable(0);
		}

		switch (m_c)
		{
		case MODE_REGISTER_HOLD:
			// 1Hz data out pulse
			m_timer_data_out->adjust(attotime::zero, 0, attotime::from_hz((clock() / 32768.0) * 2.0));

			// 64Hz time pulse
			m_timer_tp->adjust(attotime::zero, 0, attotime::from_hz((clock() / 512.0) * 2.0));
			break;

		case MODE_SHIFT:
			// enable time counter
			if (!m_testmode)
				m_timer_clock->enable(1);

			// data out LSB of shift register
			m_timer_data_out->enable(0);
			m_data_out = m_shift_reg[0] & 1;
			m_write_data(get_data_out());

			// 32Hz time pulse in testmode
			if (m_testmode)
				m_timer_tp->adjust(attotime::zero, 0, attotime::from_hz((clock() / 1024.0) * 2.0));

			break;

		case MODE_TIME_SET:
		{
			// disable time counter
			m_timer_clock->enable(0);

			// data out LSB of shift register
			m_timer_data_out->enable(0);
			m_data_out = m_shift_reg[0] & 1;
			m_write_data(get_data_out());

			// load shift register data into time counter
			int max_shift = is_serial_mode() ? 6 : 5;
			for (int i = 0; i < max_shift; i++)
				m_time_counter[i] = m_shift_reg[i];

			set_time(false,
				bcd_to_integer(m_time_counter[5]),
				m_time_counter[4] >> 4,
				bcd_to_integer(m_time_counter[3]),
				(m_time_counter[4] & 0xf) + 1,
				bcd_to_integer(m_time_counter[2]),
				bcd_to_integer(m_time_counter[1]),
				bcd_to_integer(m_time_counter[0])
			);

			// reset stage 10-15 of clock divider
			m_timer_clock->adjust(attotime::from_ticks(m_timer_clock->remaining().as_ticks(clock()) % (clock() / 512), clock()), 0, attotime::from_hz(clock() / 32768.0));

			// disable(low) time pulse in testmode
			if (m_testmode)
			{
				m_timer_tp->enable(0);
				m_tp = 0;
				m_write_tp(m_tp);
			}

			break;
		}

		case MODE_TIME_READ:
		{
			// enable time counter
			if (!m_testmode)
				m_timer_clock->enable(1);

			// load time counter data into shift register
			int max_shift = is_serial_mode() ? 6 : 5;
			for (int i = 0; i < max_shift; i++)
				m_shift_reg[i] = m_time_counter[i];

			// data out pulse: uPD4990A: 1Hz, uPD1990A: 512Hz in testmode, 0.5Hz in normal mode
			double div;
			if (m_variant == TYPE_4990A)
				div = 32768.0;
			else if (m_testmode)
				div = 64.0;
			else div = 65536.0;

			m_timer_data_out->adjust(attotime::zero, 0, attotime::from_hz((clock() / div) * 2.0));

			// 32Hz time pulse in testmode
			if (m_testmode)
				m_timer_tp->adjust(attotime::zero, 0, attotime::from_hz((clock() / 1024.0) * 2.0));

			break;
		}

		case MODE_TP_64HZ:
		case MODE_TP_256HZ:
		case MODE_TP_2048HZ:
		case MODE_TP_4096HZ:
		{
			// set timer pulse
			const double div[4] = { 512.0, 128.0, 16.0, 8.0 };
			m_timer_tp->adjust(attotime::zero, 0, attotime::from_hz((clock() / div[m_c - MODE_TP_64HZ]) * 2.0));

			break;
		}

		case MODE_TP_1S_INT:
		case MODE_TP_10S_INT:
		case MODE_TP_30S_INT:
		case MODE_TP_60S_INT:
		{
			// set timer pulse
			attotime one_second = attotime::from_hz(clock() / 32768.0);
			const double mul[4] = { 1.0, 10.0, 30.0, 60.0 };
			m_timer_tp->adjust(attotime::zero, 0, one_second * mul[m_c - MODE_TP_1S_INT] / 2.0);

			break;
		}

		case MODE_INT_RESET_OUTPUT:
		case MODE_INT_RUN_CLOCK:
		case MODE_INT_STOP_CLOCK:
			// TODO
			break;

		case MODE_TEST:
		{
			// disable time counter
			m_timer_clock->enable(0);

			// disable data out pulse
			m_timer_data_out->enable(0);

			// enable testmode
			m_testmode = true;
			m_timer_test_mode->enable(1);
			const float div = (m_variant == TYPE_4990A) ? 4.0 : 32.0; // uPD4990A: 8192Hz, uPD1990A: 1024Hz
			m_timer_test_mode->adjust(attotime::zero, 0, attotime::from_hz(clock() / div));
			break;
		}

		default:
			break;
		}
	}

	m_stb = state;
}


//-------------------------------------------------
//  clk_w -
//-------------------------------------------------

WRITE_LINE_MEMBER( upd1990a_device::clk_w )
{
	if (!m_cs)
		return;

	if (LOG) logerror("uPD1990A '%s' CLK %u\n", tag().c_str(), state);

	// rising edge
	if (!m_clk && state)
	{
		int in = m_data_in;

		if (is_serial_mode())
		{
			// always clock serial command register
			in = m_shift_reg[6] & 1;
			m_shift_reg[6] >>= 1;
			m_shift_reg[6] |= (m_data_in << 3);
		}

		if (m_c == MODE_SHIFT)
		{
			// clock shift register
			int max_shift = is_serial_mode() ? 6 : 5;
			for (int i = 0; i < max_shift; i++)
			{
				m_shift_reg[i] >>= 1;
				if (i == (max_shift - 1))
					m_shift_reg[i] |= (in << 7); // shift in new bit
				else
					m_shift_reg[i] |= (m_shift_reg[i + 1] << 7 & 0x80);
			}

			// data out LSB of shift register
			m_data_out = m_shift_reg[0] & 1;
			m_write_data(get_data_out());
		}
	}

	m_clk = state;
}


//-------------------------------------------------
//  misc input pins
//-------------------------------------------------

WRITE_LINE_MEMBER( upd1990a_device::cs_w )
{
	// chip select
	if (LOG) logerror("uPD1990A '%s' CS %u\n", tag().c_str(), state);
	m_cs = state;
}

WRITE_LINE_MEMBER( upd1990a_device::oe_w )
{
	// output enable
	if (LOG) logerror("uPD1990A '%s' OE %u\n", tag().c_str(), state);

	int prev_oe = m_oe;
	m_oe = state;

	if (m_oe != prev_oe && m_c != MODE_TEST)
		m_write_data(get_data_out());
}

WRITE_LINE_MEMBER( upd1990a_device::c0_w )
{
	if (LOG) logerror("uPD1990A '%s' C0 %u\n", tag().c_str(), state);
	m_c_unlatched = (m_c_unlatched & 0x06) | state;
}

WRITE_LINE_MEMBER( upd1990a_device::c1_w )
{
	if (LOG) logerror("uPD1990A '%s' C1 %u\n", tag().c_str(), state);
	m_c_unlatched = (m_c_unlatched & 0x05) | (state << 1);
}

WRITE_LINE_MEMBER( upd1990a_device::c2_w )
{
	if (LOG) logerror("uPD1990A '%s' C2 %u\n", tag().c_str(), state);
	m_c_unlatched = (m_c_unlatched & 0x03) | (state << 2);
}

WRITE_LINE_MEMBER( upd1990a_device::data_in_w )
{
	// data input
	if (LOG) logerror("uPD1990A '%s' DATA IN %u\n", tag().c_str(), state);
	m_data_in = state;
}


//-------------------------------------------------
//  output pins
//-------------------------------------------------

int upd1990a_device::get_data_out()
{
	// except when in testmode, data_out is high impedance when OE is low
	return (m_oe || m_testmode) ? m_data_out : 1;
}


READ_LINE_MEMBER( upd1990a_device::data_out_r )
{
	return get_data_out();
}

READ_LINE_MEMBER( upd1990a_device::tp_r )
{
	return m_tp;
}
