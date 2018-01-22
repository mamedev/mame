// license:BSD-3-Clause
// copyright-holders:Joakim Larsson Edstrom
/*********************************************************************

    Motorola MC14411 Bit Rate Generator

    This CMOS device utilizes a frequency divider network to provide
    sixteen different clock outputs (50% duty cycle). A 1.8432 MHz
    crystal oscillator normally provides the clock source for the
    network, though an external clock may also be applied to pin 21.
    A 2-bit address is used to select one of four rate multipliers
    for the F1-F14 outputs (X64, X16, X8, X1). The undivided input
    frequency is output on F16 (buffered).

    The base dividers for the F12 and F13 outputs do not go into
    1.8432 * 10^6 evenly; the frequencies shown in the table below
    have accordingly been rounded off to five figures. The X64
    frequencies for these two outputs are given on the Motorola
    datasheet as 8613.2 and 7035.5, probably incorrectly calculated.

    To lighten the burden on MAME's scheduler, the emulation of this
    device assumes that not all timer outputs will be needed at any
    given time, and only emulates the clocks actually in use. The
    special enable method intended to be used when several outputs
    are multiplexed through external switches.

*********************************************************************/

#include "emu.h"
#include "mc14411.h"

/***************************************************************************
    MACROS
***************************************************************************/

//#define LOG_GENERAL (1U <<  0) // Already defined in logmacro.h
#define LOG_SETUP   (1U <<  1)

//#define VERBOSE  (LOG_GENERAL|LOG_SETUP)
//#define LOG_OUTPUT_STREAM std::cout

#include "logmacro.h"

//#define LOG(...) LOGMASKED(LOG_GENERAL,   __VA_ARGS__) // Already defined in logmacro.h
#define LOGSETUP(...) LOGMASKED(LOG_SETUP,   __VA_ARGS__)

#ifdef _MSC_VER
#define FUNCNAME __func__
#else
#define FUNCNAME __PRETTY_FUNCTION__
#endif


/***************************************************************************
    INTERNAL TABLES
***************************************************************************/

const int mc14411_device::s_counter_divider[16] = {
		 ////////// X64 /////// X16 /////// X8 //////// X1 ////////
	  3, // F1:     614.4 kHz   153.6 kHz   76800 Hz    9600 Hz
	  4, // F2:     460.8 kHz   115.2 kHz   57600 Hz    7200 Hz
	  6, // F3:     307.2 kHz   76800 Hz    36400 Hz    4800 Hz
	  8, // F4:     230.4 kHz   57600 Hz    28800 Hz    3600 Hz
	 12, // F5:     153.6 kHz   38400 Hz    19200 Hz    2400 Hz
	 16, // F6:     115.2 kHz   28800 Hz    14400 Hz    1800 Hz
	 24, // F7:     76800 Hz    19200 Hz    9600 Hz     1200 Hz
	 48, // F8:     38400 Hz    9600 Hz     4800 Hz     600 Hz
	 96, // F9:     19200 Hz    4800 Hz     2400 Hz     300 Hz
	144, // F10:    12800 Hz    3200 Hz     1600 Hz     200 Hz
	192, // F11:    9600 Hz     2400 Hz     1200 Hz     150 Hz
	214, // F12:    8613.1 Hz   2153.3 Hz   1076.6 Hz   134.58 Hz
	262, // F13:    7035.1 Hz   1758.8 Hz   879.39 Hz   109.92 Hz
	384, // F14:    4800.0 Hz   1200 Hz     600 Hz      75 Hz
	  2, // F15:    921.6 kHz   921.6 kHz   921.6 kHz   921.6 kHz
	  1  // F16:    1.8432 MHz  1.8432 MHz  1.8432 MHz  1.8432 MHz
};

const int mc14411_device::s_divider_select[4] = {
	64, // RSB=0, RSA=0 - X1
	 8, // RSB=0, RSA=1 - X8
	 4, // RSB=1, RSA=0 - X16
	 1  // RSB=1, RSA=1 - X64
};


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

// device type definition
DEFINE_DEVICE_TYPE(MC14411, mc14411_device, "mc14411", "MC14411 Bit Rate Generator")


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

mc14411_device::mc14411_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: mc14411_device(mconfig, MC14411, tag, owner, clock)
{
}

mc14411_device::mc14411_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
	, m_out_fx_cbs{*this, *this, *this, *this, *this, *this, *this, *this, *this, *this, *this, *this, *this, *this, *this, *this }
	, m_divider(0)
	, m_reset(CLEAR_LINE)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void mc14411_device::device_start()
{
	LOGSETUP("%s\n", FUNCNAME);

	for (int i = TIMER_F1; i <= TIMER_F16; i++)
	{
		m_out_fx_cbs[i].resolve();
		m_fx_timer[i] = timer_alloc(i);
		m_timer_enabled[i] = !m_out_fx_cbs[i].isnull();
	}

	save_item(NAME(m_divider));
	save_item(NAME(m_reset));
	save_item(NAME(m_timer_enabled));

	m_reset_timer = timer_alloc(TIMER_ID_RESET);
}


//-------------------------------------------------
//  device_clock_changed - notifier called upon
//  establishment or alteration of the device clock
//-------------------------------------------------

void mc14411_device::device_clock_changed()
{
	for (int i = TIMER_F1; i <= TIMER_F16; i++)
	{
		if (m_timer_enabled[i])
			arm_timer(i);
	}
}


//-------------------------------------------------
//  timer_enable - enable or disable one timer
//-------------------------------------------------

void mc14411_device::timer_enable(timer_id i, bool enable)
{
	assert(i >= TIMER_F1 && i <= TIMER_F16);
	m_timer_enabled[i] = enable;

	if (!enable)
		m_fx_timer[i]->enable(false);
	else if (!m_fx_timer[i]->enabled())
		arm_timer(i);
}

void mc14411_device::timer_disable_all()
{
	for (int i = TIMER_F1; i <= TIMER_F16; i++)
		timer_enable((timer_id) i, false);
}

//-------------------------------------------------
//  arm_timer - arm the timer based on selected
//  divider and crystal value
//-------------------------------------------------

void mc14411_device::arm_timer(int i)
{
	assert(!m_out_fx_cbs[i].isnull());

	int divider = s_counter_divider[i];
	if (i < TIMER_F15)
		divider *= s_divider_select[m_divider];
	attotime half_cycle = clocks_to_attotime(divider) / 2; // 2 flanks per cycle
	m_fx_timer[i]->adjust(half_cycle, i, half_cycle);
	LOGSETUP(" - arming timer for F%d at %fHz (/%d)\n", i + 1, double(clock()) / divider, divider);
}


//------------------------------------------------------------------------
//  device_reset - is called by the mame framework or by the owning device
//  driver or by ASSERTING the reset line through set_reset_line
//------------------------------------------------------------------------

void mc14411_device::device_reset()
{
	LOGSETUP("%s\n", FUNCNAME);

	for (int i = TIMER_F1; i <= TIMER_F16; i++)
	{
		if (!m_out_fx_cbs[i].isnull())
		{
			// Reset line according to datasheet and remember it for transitions to come
			(m_out_fx_cbs[i])(m_fx_state[i] = (i < TIMER_F15 ? 0 : 1));
		}
	}

	if (m_reset == ASSERT_LINE)
	{
		m_reset_timer->adjust(attotime::from_nsec((double)900), TIMER_ID_RESET, attotime::from_nsec((double)900));
	}
}


//-------------------------------------------------
//  device_timer - handler timer events
//-------------------------------------------------

void mc14411_device::device_timer(emu_timer &timer, device_timer_id id, int32_t param, void *ptr)
{
	if (id >= TIMER_F1 && id <= TIMER_F16)
	{
		(m_out_fx_cbs[id])(m_fx_state[id]++ & 1);
	}
	else if (id == TIMER_ID_RESET)
	{
		// NOTE: This check could be triggered by either faulty hardware design or non accurate emulation so is just informative if the reset line is handled
		//       explicitelly instead of relying on calling device_reset
		if (!(m_reset == ASSERT_LINE))
		{
			LOG("Reset pulse is too short, should be 900nS minimum");
			logerror("Reset pulse is too short, should be 900nS minimum");
		}
	}
	else
	{
		LOG("Unhandled Timer ID %d\n", id);
	}
}


//--------------------------------------------------------
//  rate_select_w - implements the RSA and RSB input pins
// TODO: Needs to check real device behaviour how changing
//       divider at run time affects wave forms
//--------------------------------------------------------

WRITE8_MEMBER(mc14411_device::rate_select_w)
{
	LOGSETUP("%s %02x\n", FUNCNAME, data);

	if (m_divider != (data & 3))
	{
		m_divider = data & 3;
		notify_clock_changed();
	}
}


//------------------------------------------------
//  rsa_w - set RSA input line
//------------------------------------------------

WRITE_LINE_MEMBER(mc14411_device::rsa_w)
{
	LOGSETUP("%s %02x\n", FUNCNAME, state);

	if ((m_divider & RSA) != (state == ASSERT_LINE ? RSA : 0))
	{
		m_divider = (m_divider & ~RSA) | (state == ASSERT_LINE ? RSA : 0);
		notify_clock_changed();
	}
}


//------------------------------------------------
//  rsb_w - set RSB input line
//------------------------------------------------

WRITE_LINE_MEMBER(mc14411_device::rsb_w)
{
	LOGSETUP("%s %02x\n", FUNCNAME, state);

	if ((m_divider & RSB) != (state == ASSERT_LINE ? RSB : 0))
	{
		m_divider = (m_divider & ~RSB) | (state == ASSERT_LINE ? RSB : 0);
		notify_clock_changed();
	}
}


//------------------------------------------------
//  reset_w - software controlled reset
//------------------------------------------------

WRITE_LINE_MEMBER(mc14411_device::reset_w)
{
	LOGSETUP("%s %02x\n", FUNCNAME, state);

	m_reset = state;

	if (m_reset == ASSERT_LINE)
	{
		LOGSETUP(" - Asserting reset\n");
		device_reset();
	}
	else
	{
		LOGSETUP(" - Clearing reset\n");
	}
}
