// license:BSD-3-Clause
// copyright-holders:AJR
/***************************************************************************

    National Semiconductor MM5307 Baud Rate Generator/Programmable Divider

    The MM5307 divides the frequency of a crystal oscillator or external
    clock (maximum 1 MHz, ideally 921.6 kHz) to generate a single baud
    rate output. (The undivided frequency is also output on another pin,
    not emulated here.)

    The four control inputs (A, B, C, D) select one of 15 mask-programmed
    divisors or an independent external frequency. When the selected
    divisor is 2N, the output has a duty cycle of exactly 50%. When the
    selected divisor is 2N + 1, the high output is always one clock cycle
    longer than the low output. When the selected divisor is 2N + 0.5,
    the duty cycle varies slightly from phase to phase.

    The MM5307 resets its output frequency when the reset input is pulsed
    low or the control inputs change.

***************************************************************************/

#include "emu.h"
#include "mm5307.h"

#define VERBOSE 0
#include "logmacro.h"

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// device type definitions
DEFINE_DEVICE_TYPE(MM5307AA, mm5307aa_device, "mm5307aa", "MM5307AA Baud Rate Generator")
DEFINE_DEVICE_TYPE(MM5307AB, mm5307ab_device, "mm5307ab", "MM5307AB Baud Rate Generator")


//**************************************************************************
//  DIVISOR TABLES
//**************************************************************************

const std::array<u16, 16> mm5307aa_device::s_divisors_x2 = {
	0,      // external frequency (not divided)
	2304,   // 50 baud (divide by 1152)
	1536,   // 75 baud (divide by 768)
	1048,   // 110 baud (divide by 524)
	857,    // 134.5 baud (divide by 428.5)
	768,    // 150 baud (divide by 384)
	384,    // 300 baud (divide by 192)
	192,    // 600 baud (divide by 96)
	128,    // 900 baud (divide by 64)
	96,     // 1200 baud (divide by 48)
	64,     // 1800 baud (divide by 32)
	48,     // 2400 baud (divide by 24)
	32,     // 3600 baud (divide by 16)
	24,     // 4800 baud (divide by 12)
	16,     // 7200 baud (divide by 8)
	12      // 9600 baud (divide by 6)
};


const std::array<u16, 16> mm5307ab_device::s_divisors_x2 = {
	0,      // external frequency (not divided)
	2304,   // 50 baud (divide by 1152)
	576,    // 200 baud (divide by 288)
	1048,   // 110 baud (divide by 524)
	857,    // 134.5 baud (divide by 428.5)
	768,    // 150 baud (divide by 384)
	384,    // 300 baud (divide by 192)
	192,    // 600 baud (divide by 96)
	128,    // 900 baud (divide by 64)
	96,     // 1200 baud (divide by 48)
	64,     // 1800 baud (divide by 32)
	48,     // 2400 baud (divide by 24)
	32,     // 3600 baud (divide by 16)
	24,     // 4800 baud (divide by 12)
	1536,   // 75 baud (divide by 768)
	12      // 9600 baud (divide by 6)
};


//**************************************************************************
//  DEVICE IMPLEMENTATION
//**************************************************************************

//-------------------------------------------------
//  mm5307_device - constructor
//-------------------------------------------------

mm5307_device::mm5307_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, const std::array<u16, 16> &divisors_x2)
	: device_t(mconfig, type, tag, owner, clock)
	, m_divisors_x2(divisors_x2)
	, m_output_cb(*this)
	, m_ext_freq(0)
	, m_freq_control(0)
	, m_phase(0)
	, m_periodic_timer(nullptr)
{
}


//-------------------------------------------------
//  mm5307aa_device - constructor
//-------------------------------------------------

mm5307aa_device::mm5307aa_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: mm5307_device(mconfig, MM5307AA, tag, owner, clock, s_divisors_x2)
{
}


//-------------------------------------------------
//  mm5307ab_device - constructor
//-------------------------------------------------

mm5307ab_device::mm5307ab_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: mm5307_device(mconfig, MM5307AB, tag, owner, clock, s_divisors_x2)
{
}


//-------------------------------------------------
//  device_resolve_objects - resolve objects that
//  may be needed for other devices to set
//  initial conditions at start time
//-------------------------------------------------

void mm5307_device::device_resolve_objects()
{
	// Resolve callback
	m_output_cb.resolve_safe();
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void mm5307_device::device_start()
{
	// Create timer
	m_periodic_timer = timer_alloc(FUNC(mm5307_device::periodic_update), this);

	// Register for saving
	save_item(NAME(m_freq_control));
	save_item(NAME(m_phase));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void mm5307_device::device_reset()
{
	// Output delay from reset
	m_periodic_timer->adjust(attotime::from_usec(500) + clocks_to_attotime(4));
}


//-------------------------------------------------
//  control_w - set frequency control
//-------------------------------------------------

void mm5307_device::control_w(u8 data)
{
	data &= 15;
	if (data != m_freq_control)
	{
		m_freq_control = data;

		if (m_divisors_x2[data] == 0)
			LOG("%s: External frequency selected\n", machine().describe_context());
		else
			LOG("%s: %.1f baud selected\n", machine().describe_context(), clock() / (8.0 * m_divisors_x2[data]));

		// Emulate access time
		m_periodic_timer->adjust(attotime::from_usec(2800) + clocks_to_attotime(13));
	}
}


//-------------------------------------------------
//  periodic_update - update output state and
//  reset timer for next period
//-------------------------------------------------

TIMER_CALLBACK_MEMBER(mm5307_device::periodic_update)
{
	// Up to four different phases
	m_phase = (m_phase + 1) & 3;
	m_output_cb(BIT(m_phase, 0));

	u16 divisor = m_divisors_x2[m_freq_control];
	if (divisor == 0)
	{
		m_periodic_timer->adjust(m_ext_freq == 0 ? attotime::never : attotime::from_hz(m_ext_freq) / 2);
	}
	else switch (m_phase)
	{
	// First low output phase
	case 0:
		m_periodic_timer->adjust(clocks_to_attotime((divisor & ~2) >> 1 | (divisor & 1)) / 2);
		break;

	// First high output phase
	case 1:
		m_periodic_timer->adjust(clocks_to_attotime((divisor + 2) >> 2));
		break;

	// Second low output phase
	case 2:
		m_periodic_timer->adjust(clocks_to_attotime((divisor >> 2) + (divisor & 1)));
		break;

	// Second high output phase
	case 3:
		m_periodic_timer->adjust(clocks_to_attotime(((divisor + (divisor & 2)) >> 1) - ((divisor & 1) << 1)) / 2);
		break;
	}
}
