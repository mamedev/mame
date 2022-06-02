// license:BSD-3-Clause
// copyright-holders:AJR
/***************************************************************************

    Kawasaki Steel (Kawatetsu) KP63(A) Timer/Counter

    These macro cells provide 4 independent 16-bit down counters (reduced
    to 3 in some versions) driven by an 8-bit prescaler attached to the
    system clock. This prescaler is not fully emulated here, since its
    operations are mostly transparent, though a divide-by-4 clock output
    (SYNC) may be selected to appear on a port pin.

    Each counter has a single and optional external input (GATEn), which
    on the KP63 can only be used to gate a divide-by-4 count but can also
    be configured as an input clock on the KP63A.

    Two outputs are generated for each counter. The pulse or toggle output
    (OUTPn) has configurable polarity and can be used for 8-bit PWM. The
    strobe output (OUTSn) goes active high for 4 clock cycles when the
    counter underflows and is connected to the interrupt controller.

    Writing the initial count register (CR) and reading the current count
    are two-step processes, effective at the second write or first read.
    These must not be overlapped with each other since they share a
    temporary register.

***************************************************************************/

#include "emu.h"
#include "kp63.h"

#define VERBOSE 0
#include "logmacro.h"


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// device type definitions
DEFINE_DEVICE_TYPE(KP63_3CHANNEL, kp63_3channel_device, "kp63_3channel", "Kawasaki Steel KP63 Timer/Counter (3 channels)")
DEFINE_DEVICE_TYPE(KP63A, kp63a_device, "kp63a", "Kawasaki Steel KP63A Timer/Counter")

const char *const kp63_device::s_count_modes[4] =
{
	"one-shot",
	"continuous count",
	"WDT",
	"PWM"
};


//**************************************************************************
//  KP63 DEVICE
//**************************************************************************

//-------------------------------------------------
//  kp63_device - constructor
//-------------------------------------------------

kp63_device::kp63_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, u8 num_counters, u8 mode_mask)
	: device_t(mconfig, type, tag, owner, clock)
	, m_out_pulse_callback(*this)
	, m_out_strobe_callback(*this)
	, c_num_counters(num_counters)
	, c_mode_mask(mode_mask)
	, m_timer{0}
	, m_strobe_timer{0}
	, m_pwm_timer{0}
	, m_cr{0}
	, m_last_count{0}
	, m_count_tmp{0}
	, m_status{0}
	, m_rw_seq(0)
	, m_timer_started(0)
	, m_gate_input(0xf)
{
}


//-------------------------------------------------
//  kp63_3channel_device - constructor
//-------------------------------------------------

kp63_3channel_device::kp63_3channel_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: kp63_device(mconfig, KP63_3CHANNEL, tag, owner, clock, 3, 0x1f)
{
}


//-------------------------------------------------
//  kp63a_device - constructor
//-------------------------------------------------

kp63a_device::kp63a_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: kp63_device(mconfig, KP63A, tag, owner, clock, 4, 0x3f)
{
}


//-------------------------------------------------
//  device_resolve_objects - resolve objects that
//  may be needed for other devices to set
//  initial conditions at start time
//-------------------------------------------------

void kp63_device::device_resolve_objects()
{
	// Resolve output callbacks
	m_out_pulse_callback.resolve_all_safe();
	m_out_strobe_callback.resolve_all_safe();
}


//-------------------------------------------------
//  timer_expired - handle timed count underflow
//-------------------------------------------------

template <int N>
TIMER_CALLBACK_MEMBER(kp63_device::timer_expired)
{
	timer_pulse(N);
}


//-------------------------------------------------
//  strobe_off - handle end of strobe output
//-------------------------------------------------

template <int N>
TIMER_CALLBACK_MEMBER(kp63_device::strobe_off)
{
	m_out_strobe_callback[N](0);
}


//-------------------------------------------------
//  pwm_off - handle PWM phase change
//-------------------------------------------------

template <int N>
TIMER_CALLBACK_MEMBER(kp63_device::pwm_off)
{
	m_status[N] &= 0x7f;
	m_out_pulse_callback[N](BIT(m_status[N], 4) ? 1 : 0);
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void kp63_device::device_start()
{
	// Setup timers
	m_timer[0] = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(kp63_device::timer_expired<0>), this));
	m_strobe_timer[0] = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(kp63_device::strobe_off<0>), this));
	m_pwm_timer[0] = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(kp63_device::pwm_off<0>), this));
	m_timer[1] = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(kp63_device::timer_expired<1>), this));
	m_strobe_timer[1] = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(kp63_device::strobe_off<1>), this));
	m_pwm_timer[1] = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(kp63_device::pwm_off<1>), this));
	m_timer[2] = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(kp63_device::timer_expired<2>), this));
	m_strobe_timer[2] = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(kp63_device::strobe_off<2>), this));
	m_pwm_timer[2] = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(kp63_device::pwm_off<2>), this));
	if (c_num_counters > 3)
	{
		m_timer[3] = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(kp63_device::timer_expired<3>), this));
		m_strobe_timer[3] = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(kp63_device::strobe_off<3>), this));
		m_pwm_timer[3] = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(kp63_device::pwm_off<3>), this));
	}

	// Save state
	save_item(NAME(m_cr));
	save_item(NAME(m_last_count));
	save_item(NAME(m_count_tmp));
	save_item(NAME(m_status));
	save_item(NAME(m_rw_seq));
	save_item(NAME(m_timer_started));
	save_item(NAME(m_gate_input));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void kp63_device::device_reset()
{
	for (unsigned n = 0; n < c_num_counters; n++)
	{
		// Turn off timers
		m_timer[n]->adjust(attotime::never);
		m_strobe_timer[n]->adjust(attotime::never);
		m_pwm_timer[n]->adjust(attotime::never);

		// Reset status and count
		m_status[n] = 0;
		m_cr[n] = 0xffff;
		m_last_count[n] = 0xffff;

		// Clear outputs
		m_out_pulse_callback[n](0);
		m_out_strobe_callback[n](0);
	}

	// Clear read/write sequence for all counters
	m_rw_seq = 0;
	m_timer_started = 0;
}


//-------------------------------------------------
//  timer_pulse - change outputs and stop or
//  reload timer as count underflows
//-------------------------------------------------

void kp63_device::timer_pulse(unsigned n)
{
	// Toggle pulse output
	m_status[n] ^= 0x80;
	m_out_pulse_callback[n](BIT(m_status[n], 7) != BIT(m_status[n], 4) ? 1 : 0);

	// Begin strobe output
	m_out_strobe_callback[n](1);
	m_strobe_timer[n]->adjust(clocks_to_attotime(4));

	// Reload timer in continuous count and PWM modes
	if (BIT(m_status[n], 2))
		timer_reload(n);
	else
	{
		// Stop count at FFFF in one-shot and WDT modes
		m_last_count[n] = 0xffff;
		m_timer_started &= ~(1 << n);
	}
}


//-------------------------------------------------
//  timer_reload - reload timer from CR
//-------------------------------------------------

void kp63_device::timer_reload(unsigned n)
{
	m_timer_started |= 1 << n;

	if (BIT(m_status[n], 5) || ((m_status[n] & 0x03) == 0x03 && !BIT(m_gate_input, n)))
		m_last_count[n] = m_cr[n];
	else
	{
		unsigned prescale = BIT(m_status[n], 1) ? 4 : BIT(m_status[n], 0) ? 16 : 256;
		if ((m_status[n] & 0x0c) == 0x0c)
		{
			// PWM
			m_timer[n]->adjust(clocks_to_attotime(prescale * ((m_cr[n] & 0x00ff) + 1)));
			m_pwm_timer[n]->adjust(clocks_to_attotime(prescale * ((m_cr[n] >> 8) + 1)));
		}
		else
			m_timer[n]->adjust(clocks_to_attotime(prescale * (u32(m_cr[n]) + 1)));
	}
}


//-------------------------------------------------
//  timer_resume_count - start counting again
//-------------------------------------------------

void kp63_device::timer_resume_count(unsigned n)
{
	if (!BIT(m_status[n], 5) || ((m_status[n] & 0x03) != 0x03 || BIT(m_gate_input, n)))
	{
		unsigned prescale = BIT(m_status[n], 1) ? 4 : BIT(m_status[n], 0) ? 16 : 256;
		if ((m_status[n] & 0x0c) == 0x0c)
		{
			// PWM
			m_timer[n]->adjust(clocks_to_attotime(prescale * ((m_last_count[n] & 0x00ff) + 1)));
			m_pwm_timer[n]->adjust(clocks_to_attotime(prescale * ((m_last_count[n] >> 8) + 1)));
		}
		else
			m_timer[n]->adjust(clocks_to_attotime(prescale * (u32(m_last_count[n]) + 1)));
	}
}


//-------------------------------------------------
//  timer_get_count - obtain the instant count in
//  case of a readout or pause
//-------------------------------------------------

u16 kp63_device::timer_get_count(unsigned n) const
{
	if (!BIT(m_timer_started, n) || BIT(m_status[n], 5) || ((m_status[n] & 0x03) == 0x03 && !BIT(m_gate_input, n)))
		return m_last_count[n];
	else
	{
		unsigned prescale = BIT(m_status[n], 1) ? 4 : BIT(m_status[n], 0) ? 16 : 256;
		if ((m_status[n] & 0x0c) == 0x0c)
		{
			// PWM
			u8 ticks = attotime_to_clocks(m_timer[n]->remaining()) / prescale;
			return ticks | ((m_cr[n] - (u16(ticks) << 8)) & 0xff00);
		}
		else
			return attotime_to_clocks(m_timer[n]->remaining()) / prescale;
	}
}


//-------------------------------------------------
//  read - read count or status register
//-------------------------------------------------

u8 kp63_device::read(offs_t offset)
{
	const unsigned n = offset >> 1;
	assert(n < c_num_counters);

	if (BIT(offset, 0))
	{
		// Status read clears read/write sequence
		if (!machine().side_effects_disabled())
			m_rw_seq &= ~(1 << n);
		return m_status[n];
	}
	else if (BIT(m_rw_seq, n))
	{
		// Second step of counter readout
		if (!machine().side_effects_disabled())
			m_rw_seq &= ~(1 << n);
		return m_count_tmp[n];
	}
	else
	{
		// First step of counter readout
		u16 count = timer_get_count(n);
		if (!machine().side_effects_disabled())
		{
			// Latch high byte into TMP register
			m_rw_seq |= 1 << n;
			m_count_tmp[n] = count >> 8;
		}
		return count & 0x00ff;
	}
}

//-------------------------------------------------
//  write - set CR or mode register
//-------------------------------------------------

void kp63_device::write(offs_t offset, u8 data)
{
	const unsigned n = offset >> 1;
	assert(n < c_num_counters);

	if (BIT(offset, 0))
	{
		bool old_outp = BIT(m_status[n], 7) != BIT(m_status[n], 4);

		// Stop count before setting mode
		if (BIT(m_timer_started, n))
		{
			if (!BIT(m_status[n], 5) || ((m_status[n] & 0x03) != 0x03 || BIT(m_gate_input, n)))
			{
				m_last_count[n] = timer_get_count(n);
				m_timer[n]->adjust(attotime::never);
				m_pwm_timer[n]->adjust(attotime::never);
			}
			m_timer_started &= ~(1 << n);
		}

		if (BIT(data & c_mode_mask, 5))
			LOG("%s: Timer #%d configured for %s mode, %s edges of GATE, initial output %c\n",
				machine().describe_context(),
				n,
				s_count_modes[BIT(data, 2, 2)],
				BIT(data, 1) ? "???" : BIT(data, 0) ? "falling" : "rising",
				BIT(data, 4) ? 'H' : 'L');
		else
			LOG("%s: Timer #%d configured for %s mode, 1/%d system clock (GATE %s), initial output %c\n",
				machine().describe_context(),
				n,
				s_count_modes[BIT(data, 2, 2)],
				BIT(data, 1) ? 4 : BIT(data, 0) ? 16 : 256,
				(data & 0x03) == 0x03 ? "effective" : "ignored",
				BIT(data, 4) ? 'H' : 'L');
		m_status[n] = data & c_mode_mask;

		// Update OUTP
		if (old_outp != BIT(data, 4))
			m_out_pulse_callback[n](BIT(data, 4) ? 1 : 0);
	}
	else if ((m_status[n] & 0x0c) == 0x08)
	{
		// WDT retrigger (data ignored; initial count must be written using a different mode)
		timer_reload(n);
	}
	else if (BIT(m_rw_seq, n))
	{
		// Second step of initial count write
		m_rw_seq &= ~(1 << n);
		m_cr[n] = u16(data) << 8 | m_count_tmp[n];

		LOG("%s: Timer #%d initial count = %d\n", machine().describe_context(), n, (m_status[n] == 0x0c) ? m_cr[n] & 0x00ff : m_cr[n]);

		// Automatic retrigger in one-shot and continuous modes
		if (!BIT(m_status[n], 3) || !BIT(m_timer_started, n))
		{
			if (!BIT(m_status[n], 7))
			{
				// Toggle OUTP
				m_status[n] |= 0x80;
				m_out_pulse_callback[n](BIT(m_status[n], 4) ? 0 : 1);
			}
			timer_reload(n);
		}
	}
	else
	{
		// First step of initial count write (held in TMP register)
		m_rw_seq |= 1 << n;
		m_count_tmp[n] = data;
	}
}

//-------------------------------------------------
//  write_gate - handle gate inputs
//-------------------------------------------------

void kp63_device::write_gate(unsigned n, bool state)
{
	assert(n < c_num_counters);

	if (BIT(m_gate_input, n) != state)
		return;

	if (state)
		m_gate_input |= 1 << n;
	else
		m_gate_input &= ~(1 << n);

	if (BIT(m_timer_started, n))
	{
		if ((m_status[n] & 0x23) == 0x03)
		{
			// Timer gated on or off
			if (state)
				timer_resume_count(n);
			else
			{
				m_last_count[n] = timer_get_count(n);
				m_timer[n]->adjust(attotime::never);
			}
		}
		else if ((m_status[n] & 0x23) == (state ? 0x21 : 0x20))
		{
			// Count edges of gate input
			if ((m_status[n] & 0x0c) == 0x0c)
			{
				// PWM: count is in lower 8 bits
				if ((m_last_count[n] & 0x00ff) == 0)
					timer_pulse(n);
				else
				{
					// Decrement both halves and check for underflow in upper half
					m_last_count[n] -= 0x0101;
					if (m_last_count[n] >= 0xff00)
					{
						m_status[n] &= 0x7f;
						m_out_pulse_callback[n](BIT(m_status[n], 4) ? 1 : 0);
					}
				}
			}
			else if (m_last_count[n]-- == 0)
				timer_pulse(n);
		}
	}
}
