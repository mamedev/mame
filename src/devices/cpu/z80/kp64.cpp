// license:BSD-3-Clause
// copyright-holders:AJR
/***************************************************************************

    Kawasaki Steel (Kawatetsu) KP64 Timer/Counter Unit

    This is a macro cell providing one 16-bit counter/timer with four
    operating modes. KL5C80A12 allows two of these to be cascaded as one
    32-bit counter, but this option is scarcely documented and has not been
    emulated here.

    Either the system clock (CLK) or the falling edge of an external clock
    (XCLK) can be selected as the count source for any mode of operation.
    The output (OUT) is initialized to L after reset and for most mode
    settings, after which it may either toggle or pulse high depending on
    the mode setting. In pulse mode, the OUT polarity is selectable. OUT
    is also connected to the KL5C80A12 interrupt controller.

    Each time a mode control word is written, a new 16-bit value must be
    provided for the CR register except in pulse width/frequency
    measurement mode, which instead uses CR to hold the measured count.
    Since the CPU can only write to registers 8 bits at a time, a separate
    8-bit holding register (TMP) is used to prevent CR from being updated
    until both lower and higher bytes have been written. Likewise, the
    counter value can only be read 8 bits at a time, and a stable readout
    is guaranteed by requiring the count to be first latched into the
    OR register by a command. Another command is used to reset the read/
    write sequence to the lower byte state.

    In the frequency divide and PWM modes, the counter is loaded when the
    first count value is written and automatically reloaded on each
    subsequent underflow. In the pulse mode, counting stops after
    underflow, and the counter is loaded and started or restarted whenever
    a new count value is written in the soft trigger submode, or following
    the rising edge of the GATE input in the hard trigger submode. In the
    pulse width/frequency measurement mode, the counter is loaded with
    FFFFH after either the rising or falling edge of the GATE input, and
    another selectable GATE edge completes the measurement and loads the
    complement of the count value into CR. Counting continues after
    measurement is complete if continuous measurement is selected.

    For all modes which do not use GATE as a trigger to start or restart
    counting, counting is enabled when the GATE input is at a high level.
    The H and L periods of GATE must be at least two system clock cycles
    wide.

***************************************************************************/

#include "emu.h"
#include "kp64.h"

#define VERBOSE 0
#include "logmacro.h"

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// device type definition
DEFINE_DEVICE_TYPE(KP64, kp64_device, "kp64", "Kawasaki Steel KP64 Timer/Counter")


//**************************************************************************
//  KP64 DEVICE
//**************************************************************************

//-------------------------------------------------
//  kp64_device - constructor
//-------------------------------------------------

kp64_device::kp64_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, KP64, tag, owner, clock)
	, m_out_callback(*this)
	, m_count_timer(nullptr)
	, m_pulse_timer(nullptr)
	, m_xclk(true)
	, m_gate(true)
	, m_count(0)
	, m_cr(0)
	, m_or(0)
	, m_tmp(0)
	, m_status(0x40)
	, m_read_msb(false)
	, m_write_msb(false)
	, m_reload(false)
	, m_started(false)
{
}


//-------------------------------------------------
//  device_resolve_objects - resolve objects that
//  may be needed for other devices to set
//  initial conditions at start time
//-------------------------------------------------

void kp64_device::device_resolve_objects()
{
	// Resolve output callback
	m_out_callback.resolve_safe();
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void kp64_device::device_start()
{
	// Setup timers
	m_count_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(kp64_device::count_underflow), this));
	m_pulse_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(kp64_device::pulse_off), this));

	// Save state
	save_item(NAME(m_xclk));
	save_item(NAME(m_gate));
	save_item(NAME(m_count));
	save_item(NAME(m_cr));
	save_item(NAME(m_or));
	save_item(NAME(m_tmp));
	save_item(NAME(m_status));
	save_item(NAME(m_read_msb));
	save_item(NAME(m_write_msb));
	save_item(NAME(m_reload));
	save_item(NAME(m_started));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void kp64_device::device_reset()
{
	// Stop timers
	m_count_timer->enable(false);
	m_pulse_timer->enable(false);

	// Clear all registers
	m_count = 0xffff;
	m_cr = 0xffff;
	m_or = 0xffff;
	m_status = 0x01; // system clock synchronous
	m_read_msb = false;
	m_write_msb = false;
	m_reload = false;
	m_started = false;

	// Set output low
	set_out(false);
}


//-------------------------------------------------
//  set_out - update OUT and status register
//-------------------------------------------------

void kp64_device::set_out(bool state)
{
	if (BIT(m_status, 6) != state)
	{
		LOG("%s: OUT = %c\n", machine().time().to_string(), state ? 'H' : 'L');
		if (state)
			m_status |= 0x40;
		else
			m_status &= 0xbf;
		m_out_callback(state);
	}
}


//-------------------------------------------------
//  count_value - get current counter value
//-------------------------------------------------

u16 kp64_device::count_value() const noexcept
{
	if (m_count_timer->enabled())
		return std::min<u32>(attotime_to_clocks(m_count_timer->remaining()), 0xffff);
	else
		return m_count;
}


//-------------------------------------------------
//  reload_count - reload and begin counting
//-------------------------------------------------

void kp64_device::reload_count()
{
	m_count = BIT(m_status, 5) ? 0xffff : m_cr;
	if (BIT(m_status, 0))
	{
		// hng64 network MCU configures this supposedly invalid value and thrashes the scheduler if the timer is enabled
		if (m_count == 0)
			logerror("%s: Zero reload value specified for timer\n", machine().describe_context());
		else
			m_count_timer->adjust(clocks_to_attotime(u32(m_count) + 1));
	}

	// Count is now started whether or not it was before
	m_reload = false;
	m_started = true;

	switch (BIT(m_status, 3, 3))
	{
	case 0b000:
		// Set OUT low before pulse
		if (BIT(m_status, 2))
			set_out(false);
		break;

	case 0b001:
		// Initiate H phase of PWM mode
		set_out(true);
		break;

	case 0b010:
		// One-shot pulse mode
		set_out(BIT(m_status, 1));
		break;

	case 0b011:
		// Strobe pulse mode
		set_out(!BIT(m_status, 1));
		break;
	}
}


//-------------------------------------------------
//  finish_count - handle counter decrement from 0
//-------------------------------------------------

void kp64_device::finish_count()
{
	if (BIT(m_status, 5))
	{
		// Count merely wraps around in pulse width/frequency measurement mode
		m_count = 0xffff;
		if (BIT(m_status, 0))
			m_count_timer->adjust(clocks_to_attotime(u32(m_count) + 1));
	}
	else if (BIT(m_status, 4))
	{
		// Toggle output
		set_out(!BIT(m_status, 6));
		if (BIT(m_status, 0) && BIT(m_status, 3))
			m_pulse_timer->adjust(clocks_to_attotime(1));

		// Wait for retrigger
		m_started = false;
	}
	else if (BIT(m_status, 3))
	{
		// Alternating reload for PWM mode
		m_count = (BIT(m_status, 6) ? ~m_cr : m_cr) & ((0x40 << (m_status & 0x06)) - 1);
		if (BIT(m_status, 0))
			m_count_timer->adjust(clocks_to_attotime(u32(m_count) + 1));

		// Toggle output
		set_out(!BIT(m_status, 6));
	}
	else
	{
		// Automatic reload
		m_count = m_cr;
		if (BIT(m_status, 0))
		{
			m_count_timer->adjust(clocks_to_attotime(u32(m_count) + 1));
			if (BIT(m_status, 2))
				m_pulse_timer->adjust(clocks_to_attotime(1));
		}

		// Pulse or toggle output
		set_out(BIT(m_status, 2) || !BIT(m_status, 6));
	}
}


//-------------------------------------------------
//  count_underflow - handle timer expiry
//-------------------------------------------------

TIMER_CALLBACK_MEMBER(kp64_device::count_underflow)
{
	finish_count();
}


//-------------------------------------------------
//  pulse_off - end strobe
//-------------------------------------------------

TIMER_CALLBACK_MEMBER(kp64_device::pulse_off)
{
	set_out(BIT(m_status, 4) && !BIT(m_status, 1));
}


//**************************************************************************
//  I/O REGISTER INTERFACE
//**************************************************************************

//-------------------------------------------------
//  counter_r - read OR or CR byte in sequence
//-------------------------------------------------

u8 kp64_device::counter_r()
{
	u8 data = (BIT(m_status, 5) ? m_cr : m_or) >> (m_read_msb ? 8 : 0);

	// Advance read sequence
	if (!machine().side_effects_disabled())
		m_read_msb = !m_read_msb;

	return data;
}


//-------------------------------------------------
//  counter_w - write count byte in sequence
//-------------------------------------------------

void kp64_device::counter_w(u8 data)
{
	if (m_write_msb)
	{
		m_cr = u16(data) << 8 | m_tmp;
		LOG("%s: %04XH entered into CR while %s\n", machine().describe_context(), m_cr, m_started ? "started" : "stopped");

		// Load count into CR for frequency divide modes and PWM modes if this was the initial count
		if (!m_started && BIT(m_status, 4, 2) == 0)
		{
			if (BIT(m_status, 0) && m_gate)
				reload_count();
			else
				m_reload = true;
		}

		m_write_msb = false;
	}
	else
	{
		m_tmp = data;
		m_write_msb = true;
	}
}


//-------------------------------------------------
//  status_r - read status word
//-------------------------------------------------

u8 kp64_device::status_r()
{
	// Bit 6 is always 0 in pulse width/frequency measurement mode
	return m_status & (BIT(m_status, 5) ? 0xbf : 0xff);
}


//-------------------------------------------------
//  control_w - write control word
//-------------------------------------------------

void kp64_device::control_w(u8 data)
{
	switch (BIT(data, 3, 3))
	{
	case 0b000:
		LOG("%s: Frequency divide mode selected (%s source, %s output)\n",
			machine().describe_context(),
			BIT(data, 0) ? "CLK" : "XCLK",
			BIT(data, 2) ? "pulse" : "toggle");

		m_status = data & 0x3d;
		m_cr = 0xffff;
		m_count = 0xffff;
		m_started = false;
		m_reload = false;
		set_out(false);
		m_count_timer->enable(false);
		m_pulse_timer->enable(false);
		break;

	case 0b001:
		LOG("%s: PWM mode selected (period = %s/%d)\n",
			machine().describe_context(),
			BIT(data, 0) ? "CLK" : "XCLK",
			(0x40 << (data & 0x06)) + 1);
		m_status = data & 0x3f;
		m_cr = 0xffff;
		m_count = (0x40 << (m_status & 0x06)) - 1;
		m_started = false;
		m_reload = false;
		set_out(false);
		m_count_timer->enable(false);
		m_pulse_timer->enable(false);
		break;

	case 0b010: case 0b011:
		LOG("%s: Pulse mode selected (%s source, %s trigger, %s%s output)\n",
			machine().describe_context(),
			BIT(data, 0) ? "CLK" : "XCLK",
			BIT(data, 2) ? "hard" : "soft",
			BIT(data, 1) ? "reverse " : "",
			BIT(data, 3) ? "strobe" : "one shot");
		m_status = data & 0x3f;
		m_cr = 0xffff;
		m_count = 0xffff;
		m_started = false;
		m_reload = false;
		set_out(!BIT(data, 1));
		m_count_timer->enable(false);
		m_pulse_timer->enable(false);
		break;

	case 0b100: case 0b101:
		LOG("%s: Pulse width/frequency measurement mode selected (%s source, %s edge to %s edge, %s)\n",
			machine().describe_context(),
			BIT(data, 0) ? "CLK" : "XCLK",
			BIT(data, 2) ? "rising" : "falling",
			BIT(data, 1) ? "rising" : "falling",
			BIT(data, 3) ? "continuous" : "once");
		m_status = data & 0x3f;
		m_cr = 0xffff;
		m_count = 0xffff;
		m_started = false;
		m_reload = false;
		set_out(false);
		m_count_timer->enable(false);
		m_pulse_timer->enable(false);
		break;

	case 0b111:
		if (BIT(data, 0, 2) == 0)
		{
			// Counter latch command
			m_or = count_value();
			LOG("%s: %04XH latched into OR\n", machine().describe_context(), m_or);
		}
		else
		{
			// Flag clear command
			if (BIT(data, 0))
			{
				if (BIT(m_status, 7))
					LOG("%s: Flag cleared\n", machine().describe_context());
				m_status &= 0x7f;
			}

			// R/W sequence clear command
			if (BIT(data, 1))
			{
				m_read_msb = false;
				m_write_msb = false;
			}
		}
		break;

	default:
		logerror("%s: Unrecognized control word %02XH written\n", machine().describe_context(), data);
		break;
	}
}


//**************************************************************************
//  INPUT LINES
//**************************************************************************

//-------------------------------------------------
//  xclk_w - set external count input
//-------------------------------------------------

WRITE_LINE_MEMBER(kp64_device::xclk_w)
{
	// Only falling edges count
	if (std::exchange(m_xclk, state) && !state)
	{
		// Ignore if system clock selected
		if (BIT(m_status, 0))
			return;

		// Ignore if gated off
		if (!m_gate && !BIT(m_status, 5) && (!BIT(m_status, 4) || !BIT(m_status, 2)))
			return;

		if (m_reload)
			reload_count();
		else if (m_started && m_count-- == 0)
			finish_count();
		else
		{
			// Terminate pulse
			if (BIT(m_status, 2, 4) == 0b0001)
				set_out(0);
			else if (BIT(m_status, 3, 3) == 0b011)
				set_out(!BIT(m_status, 1));
		}
	}
}


//-------------------------------------------------
//  gate_w - set gate input
//-------------------------------------------------

WRITE_LINE_MEMBER(kp64_device::gate_w)
{
	if (m_gate == bool(state))
		return;

	m_gate = state;
	if (BIT(m_status, 5))
	{
		if (BIT(m_status, 2) == state && (BIT(m_status, 3) || !BIT(m_status, 7)))
			m_reload = true;

		if (BIT(m_status, 1) == state && m_started)
		{
			m_cr = ~count_value();
			LOG("%s: Measurement completed (count = %04X)\n", machine().time().to_string(), m_cr);
			m_status |= 0x80;
			if (!BIT(m_status, 3))
			{
				m_started = false;
				m_reload = false;
			}

			// Pulse H for one system clock cycle
			set_out(true);
			m_pulse_timer->adjust(clocks_to_attotime(1));
		}

		if (m_reload && BIT(m_status, 0))
			reload_count();
	}
	else if (BIT(m_status, 4) && BIT(m_status, 2))
	{
		if (state)
		{
			LOG("%s: Hard trigger received\n", machine().time().to_string());
			m_status |= 0x80;
			if (BIT(m_status, 0))
				reload_count();
			else
				m_reload = true;
		}
	}
	else if (BIT(m_status, 0))
	{
		LOG("%s: Timer gated %s\n", machine().time().to_string(), state ? "on" : "off");
		if (state)
		{
			if (m_reload)
				reload_count();
			else if (m_started)
				m_count_timer->adjust(clocks_to_attotime(u32(m_count) + 1));
		}
		else if (m_started)
		{
			m_count = count_value();
			m_count_timer->enable(false);
		}
	}
}
