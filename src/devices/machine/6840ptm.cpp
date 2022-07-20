// license:BSD-3-Clause
// copyright-holders:James Wallace
/***************************************************************************

    Motorola 6840 (PTM)

    Programmable Timer Module

    Written By J.Wallace based on previous work by Aaron Giles,
   'Re-Animator' and Mathis Rosenhauer.

    Todo:
         Confirm handling for 'Single Shot' operation.
         (Datasheet suggests that output starts high, going low
         on timeout, opposite of continuous case)
         Establish whether ptm6840_set_c? routines can replace
         hard coding of external clock frequencies.


    Operation:
    The interface is arranged as follows:

    Internal Clock frequency,
    Clock 1 frequency, Clock 2 frequency, Clock 3 frequency,
    Clock 1 output, Clock 2 output, Clock 3 output,
    IRQ function

    If the external clock frequencies are not fixed, they should be
    entered as '0', and the ptm6840_set_c?(which, state) functions
    should be used instead if necessary (This should allow the VBLANK
    clock on the MCR units to operate).


    2009-06 Converted to be a device

***************************************************************************/

#include "emu.h"
#include "6840ptm.h"

#define LOG_COUNTERS    (1 << 1)
#define LOG_STATUS      (1 << 2)
#define LOG_CONTROL     (1 << 3)
#define LOG_RESETS      (1 << 4)
#define LOG_TIMEOUTS    (1 << 5)
#define LOG_IRQS        (1 << 6)
#define LOG_ALL         (LOG_COUNTERS | LOG_STATUS | LOG_CONTROL | LOG_RESETS | LOG_TIMEOUTS | LOG_IRQS)

#define VERBOSE         (0)
#include "logmacro.h"

/***************************************************************************
    LOCAL VARIABLES
***************************************************************************/

const char *const ptm6840_device::opmode[] =
{
	"000 continuous mode",
	"001 freq comparison mode",
	"010 continuous mode",
	"011 pulse width comparison mode",
	"100 single shot mode",
	"101 freq comparison mode",
	"110 single shot mode",
	"111 pulse width comparison mode"
};

/***************************************************************************
    IMPLEMENTATION
***************************************************************************/

// device type definition
DEFINE_DEVICE_TYPE(PTM6840, ptm6840_device, "ptm6840", "MC6840 PTM")

//-------------------------------------------------
//  ptm6840_device - constructor
//-------------------------------------------------

ptm6840_device::ptm6840_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, PTM6840, tag, owner, clock)
	, m_external_clock{ 0.0, 0.0, 0.0 }
	, m_out_cb(*this)
	, m_irq_cb(*this)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void ptm6840_device::device_start()
{
	// resolve callbacks
	m_out_cb.resolve_all_safe();
	m_irq_cb.resolve_safe();

	m_timer[0] = timer_alloc(FUNC(ptm6840_device::state_changed), this);
	m_timer[1] = timer_alloc(FUNC(ptm6840_device::state_changed), this);
	m_timer[2] = timer_alloc(FUNC(ptm6840_device::state_changed), this);

	for (auto & elem : m_timer)
	{
		elem->enable(false);
	}

	// register for state saving
	save_item(NAME(m_lsb_buffer));
	save_item(NAME(m_msb_buffer));
	save_item(NAME(m_status_read_since_int));
	save_item(NAME(m_status_reg));
	save_item(NAME(m_t3_divisor));
	save_item(NAME(m_t3_scaler));
	save_item(NAME(m_irq));

	save_item(NAME(m_control_reg));
	save_item(NAME(m_output));
	save_item(NAME(m_gate));
	save_item(NAME(m_clk));
	save_item(NAME(m_mode));
	save_item(NAME(m_single_fired));
	save_item(NAME(m_enabled));
	save_item(NAME(m_external_clock));
	save_item(NAME(m_counter));
	save_item(NAME(m_disable_time));
	save_item(NAME(m_latch));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void ptm6840_device::device_reset()
{
	m_control_reg[2]         = 0;
	m_control_reg[1]         = 0;
	m_control_reg[0]         = 1;
	m_status_reg             = 0;
	m_t3_divisor             = 1;
	m_status_read_since_int = 0;
	m_irq                   = 0;
	m_t3_scaler             = 0;

	for (int i = 0; i < 3; i++)
	{
		m_counter[i]      = 0xffff;
		m_latch[i]        = 0xffff;
		m_disable_time[i] = attotime::never;
		m_output[i]       = false;
		m_clk[i]          = false;
		m_single_fired[i] = false;
		m_enabled[i]      = false;
		m_mode[i]         = 0;
	}
}

void ptm6840_device::device_resolve_objects()
{
	for (int i = 0; i < 3; i++)
		m_gate[i] = false;
}


//-------------------------------------------------
//  deduct_from_counter - count back by one step
//-------------------------------------------------

void ptm6840_device::deduct_from_counter(int idx)
{
	if (m_control_reg[idx] & COUNT_MODE_8BIT)
	{
		// Dual-byte mode
		uint16_t msb = m_counter[idx] >> 8;
		uint16_t lsb = m_counter[idx] & 0xff;

		lsb--;

		bool timed_out = false;
		if (lsb == 0xffff)
		{
			// Borrow from the MSB
			lsb = (m_latch[idx] & 0xff) + 1;
			msb--;

			if (msb < 0)
			{
				// If MSB is less than zero, we've timed out, no need to manually reload
				timed_out = true;
				state_changed(idx);
			}
			else if (msb == 0)
			{
				// If MSB is at zero, our output state potentially needs to change, also no need to manually reload
				msb = (m_latch[idx] >> 8) + 1;
				state_changed(idx);
			}
		}

		// Store the result if we haven't timed out (which already reloads the counter from the latches)
		if (!timed_out)
		{
			m_counter[idx] = (msb << 8) + lsb;
		}
	}
	else
	{
		// Word mode
		m_counter[idx]--;

		const bool one_shot_mode = m_mode[idx] == 4 || m_mode[idx] == 6;
		// If we've ticked once in one-shot-mode, or we've expired, our state needs to change
		if ((one_shot_mode && !m_output[idx]) || m_counter[idx] == 0xffff)
		{
			state_changed(idx);
		}
	}
}



//-------------------------------------------------
//  update_interrupts - Update Internal Interrupts
//-------------------------------------------------

void ptm6840_device::update_interrupts()
{
	int new_state = ((m_status_reg & TIMER1_IRQ) && (m_control_reg[0] & INTERRUPT_EN)) ||
					((m_status_reg & TIMER2_IRQ) && (m_control_reg[1] & INTERRUPT_EN)) ||
					((m_status_reg & TIMER3_IRQ) && (m_control_reg[2] & INTERRUPT_EN));

	LOGMASKED(LOG_IRQS, "%s: IRQ state update: %d, T1:%d, T1E:%d, T2:%d, T2E:%d, T3:%d, T3E:%d\n", machine().describe_context(), new_state,
		(m_status_reg & TIMER1_IRQ) ? 1 : 0, (m_control_reg[0] & INTERRUPT_EN) ? 1 : 0,
		(m_status_reg & TIMER2_IRQ) ? 1 : 0, (m_control_reg[1] & INTERRUPT_EN) ? 1 : 0,
		(m_status_reg & TIMER3_IRQ) ? 1 : 0, (m_control_reg[2] & INTERRUPT_EN) ? 1 : 0);

	if (new_state != m_irq)
	{
		m_irq = new_state;

		if (m_irq)
		{
			m_status_reg |= ANY_IRQ;
		}
		else
		{
			m_status_reg &= ~ANY_IRQ;
		}

		m_irq_cb(m_irq);
	}
}



//-------------------------------------------------
//  compute_counter - Compute Counter
//-------------------------------------------------

int ptm6840_device::compute_counter(int idx) const
{
	uint32_t clk;

	// If the timer is disabled, return the raw counter value
	if (!m_enabled[idx])
	{
		LOGMASKED(LOG_COUNTERS, "Timer #%d read counter: %04x\n", idx + 1, m_counter[idx]);
		return m_counter[idx];
	}
	else if (m_control_reg[0] & RESET_TIMERS)
	{
		// If we're held in reset, return either the latch value for 16-bit mode, or the computed count for dual-8-bit
		if (m_control_reg[idx] & COUNT_MODE_8BIT)
		{
			const uint16_t latch_lsb = m_latch[idx] & 0xff;
			const uint16_t latch_msb = m_latch[idx] >> 8;
			return latch_msb * (latch_lsb + 1);
		}
		return m_latch[idx];
	}

	// determine the clock frequency for this timer
	if (m_control_reg[idx] & INTERNAL_CLK_EN)
	{
		clk = clock();
	}
	else
	{
		clk = m_external_clock[idx];
	}

	if (idx == 2)
	{
		clk /= m_t3_divisor;
	}
	LOGMASKED(LOG_COUNTERS, "Timer #%d %s clock freq %d\n", idx + 1, (m_control_reg[idx] & INTERNAL_CLK_EN) ? "internal" : "external", clk);

	// See how many are left
	attotime remaining_time = m_timer[idx]->remaining();
	if (remaining_time.is_never())
	{
		if (m_disable_time[idx].is_never())
		{
			return m_counter[idx];
		}
		remaining_time = m_disable_time[idx];
	}
	int remaining = remaining_time.as_ticks(clk);

	LOGMASKED(LOG_COUNTERS, "Timer #%d read counter: %04x\n", idx + 1, remaining);
	return remaining;
}



//-------------------------------------------------
//  reload_counter
//-------------------------------------------------

void ptm6840_device::reload_counter(int idx)
{
	const bool one_shot_mode = m_mode[idx] == 4 || m_mode[idx] == 6;

	// Copy the latched value in
	m_counter[idx] = m_latch[idx];

	// Determine the clock frequency for this timer
	double clk;
	if (m_control_reg[idx] & INTERNAL_CLK_EN)
	{
		clk = static_cast<double> (clock());
		LOGMASKED(LOG_COUNTERS, "Timer #%d internal clock freq %f \n", idx + 1, clk);
	}
	else
	{
		clk = m_external_clock[idx];
		LOGMASKED(LOG_COUNTERS, "Timer #%d external clock freq %f \n", idx + 1, clk);
	}

	// Determine the number of clock periods before we expire
	int count = m_counter[idx] + 1;
	if (m_control_reg[idx] & COUNT_MODE_8BIT)
	{
		const uint16_t latch_lsb = m_latch[idx] & 0xff;
		const uint16_t latch_msb = m_latch[idx] >> 8;
		if (!m_output[idx])
		{
			count = (latch_lsb + 1) * latch_msb;
		}
		else
		{
			count = latch_lsb + 1;
		}
	}
	else if (one_shot_mode)
	{
		if (!m_output[idx])
		{
			count = 1;
		}
		else
		{
			count = m_counter[idx];
		}
	}

	// Set the timer
	LOGMASKED(LOG_COUNTERS, "Timer #%d init_timer: clock = %f  count = %04x\n", idx + 1, clk, count);

	if (clk == 0.0)
	{
		m_enabled[idx] = false;
		m_timer[idx]->adjust(attotime::never);
	}
	else
	{
		m_enabled[idx] = true;
		attotime duration = attotime::from_hz(clk) * count;

		if (idx == 2)
		{
			duration *= m_t3_divisor;
		}

		LOGMASKED(LOG_COUNTERS, "Timer #%d init_timer: duration = %f\n", idx + 1, duration.as_double());

		const bool one_shot_mode = m_mode[idx] == 4 || m_mode[idx] == 6;
		const bool gated = (!one_shot_mode && m_gate[idx]) || (m_control_reg[0] & RESET_TIMERS);
		if (gated)
		{
			m_disable_time[idx] = duration;
			m_timer[idx]->adjust(attotime::never);
		}
		else
		{
			m_timer[idx]->adjust(duration, idx);
		}
	}
}



//-------------------------------------------------
//  read - Read Timer
//-------------------------------------------------

uint8_t ptm6840_device::read(offs_t offset)
{
	int val;

	switch ( offset )
	{
		case PTM_6840_CTRL1:
			val = 0;
			break;

		case PTM_6840_STATUS:
			LOGMASKED(LOG_STATUS, "%s: Status read = %04X\n", machine().describe_context(), m_status_reg);
			m_status_read_since_int |= m_status_reg & 0x07;
			val = m_status_reg;
			break;

		case PTM_6840_LSB1:
		case PTM_6840_LSB2:
		case PTM_6840_LSB3:
			val = m_lsb_buffer;
			LOGMASKED(LOG_COUNTERS, "%s: Counter LSB read = %02x\n", machine().describe_context(), val);
			break;

		case PTM_6840_MSBBUF1:
		case PTM_6840_MSBBUF2:
		case PTM_6840_MSBBUF3:
		{
			int idx = (offset - 2) / 2;
			uint16_t result = compute_counter(idx);

			// Clear the interrupt if the status has been read
			if (m_status_read_since_int & (1 << idx))
			{
				m_status_reg &= ~(1 << idx);
				update_interrupts();
			}

			val = result >> 8;
			m_lsb_buffer = (uint8_t)result;

			LOGMASKED(LOG_COUNTERS, "%s: Counter %d MSB read = %02x\n", machine().describe_context(), idx + 1, val);
			break;
		}

		default:
			val = 0;
			break;

	}
	return val;
}


//-------------------------------------------------
//  write - Write Timer
//-------------------------------------------------

void ptm6840_device::write(offs_t offset, uint8_t data)
{
	switch ( offset )
	{
		case PTM_6840_CTRL1:
		case PTM_6840_CTRL2:
		{
			int idx = (offset == 1) ? 1 : (m_control_reg[1] & CR1_SELECT) ? 0 : 2;
			uint8_t diffs = data ^ m_control_reg[idx];
			m_mode[idx] = (data >> 3) & 0x07;
			m_control_reg[idx] = data;
			m_t3_divisor = (m_control_reg[2] & T3_PRESCALE_EN) ? 8 : 1;

			LOGMASKED(LOG_CONTROL, "Control register #%d selected\n", idx + 1);
			LOGMASKED(LOG_CONTROL, "operation mode   = %s\n", opmode[m_mode[idx]]);
			LOGMASKED(LOG_CONTROL, "value            = %02x\n", m_control_reg[idx]);
			LOGMASKED(LOG_CONTROL, "t3divisor        = %d\n", m_t3_divisor);
			LOGMASKED(LOG_CONTROL, "irq/output/int   = %d/%d/%d\n", BIT(m_control_reg[idx], 6), BIT(m_control_reg[idx], 7), BIT(m_control_reg[idx], 1));
			LOGMASKED(LOG_CONTROL, "latch            = %04x\n", m_latch[idx]);
			LOGMASKED(LOG_CONTROL, "counter          = %04x\n", m_counter[idx]);

			if (diffs & INTERRUPT_EN)
				update_interrupts();

			if (!(m_control_reg[idx] & COUNT_OUT_EN))
			{
				// Output cleared
				m_out_cb[idx](0);
			}

			// Reset?
			if (idx == 0 && (diffs & RESET_TIMERS))
			{
				// Holding reset down
				if (data & RESET_TIMERS)
				{
					m_status_reg = 0;
					m_status_read_since_int = 0;
					update_interrupts();
					LOGMASKED(LOG_RESETS, "Timer reset\n");
					for (int i = 0; i < 3; i++)
					{
						m_timer[i]->adjust(attotime::never);
						m_enabled[i] = false;
						reload_counter(i);
						m_output[i] = false;
						m_out_cb[i](m_output[i]);
					}
				}
				// Releasing reset
				else
				{
					for (int i = 0; i < 3; i++)
					{
						m_single_fired[i] = false;
						reload_counter(i);
						if (!m_disable_time[i].is_never() && m_timer[i]->remaining().is_never() && ((m_control_reg[i] & INTERNAL_CLK_EN) || m_external_clock[i] != 0.0))
						{
							m_timer[i]->adjust(m_disable_time[i], i);
							m_disable_time[i] = attotime::never;
						}
					}
				}

				m_status_reg = 0;
				update_interrupts();
			}

			// Changing the clock source? (e.g. Zwackery)
			if (diffs & INTERNAL_CLK_EN)
			{
				update_expiration_for_clock_source(idx, !(m_control_reg[idx] & INTERNAL_CLK_EN), m_external_clock[idx]);
			}
			break;
		}

		case PTM_6840_MSBBUF1:
		case PTM_6840_MSBBUF2:
		case PTM_6840_MSBBUF3:
		{
			LOGMASKED(LOG_COUNTERS, "msbbuf%d = %02X\n", offset / 2, data);
			m_msb_buffer = data;
			break;
		}

		case PTM_6840_LSB1:
		case PTM_6840_LSB2:
		case PTM_6840_LSB3:
		{
			int idx = (offset - 3) / 2;
			m_latch[idx] = (m_msb_buffer << 8) | (data & 0xff);

			// Clear the interrupt
			m_status_reg &= ~(1 << idx);
			update_interrupts();

			// Reload the count if in an appropriate mode
			if (!(m_control_reg[idx] & 0x10) || (m_control_reg[0] & RESET_TIMERS))
			{
				reload_counter(idx);
			}

			LOGMASKED(LOG_COUNTERS, "%s: Counter #%d latch = %04X\n", machine().describe_context(), idx + 1, m_latch[idx]);
			break;
		}
	}
}


//-------------------------------------------------
//  state_changed - called if timer output state
//  changes (masked or not)
//-------------------------------------------------

TIMER_CALLBACK_MEMBER(ptm6840_device::state_changed)
{
	LOGMASKED(LOG_TIMEOUTS, "**ptm6840 t%d state_changed**\n", param + 1);

	// Set the interrupt flag if at the end of a full cycle
	const bool one_shot_mode = m_mode[param] == 4 || m_mode[param] == 6;
	const bool dual_8bit = m_control_reg[param] & COUNT_MODE_8BIT;
	const bool end_of_cycle = (!dual_8bit && !one_shot_mode) || m_output[param];
	if (end_of_cycle)
	{
		m_status_reg |= (1 << param);
		m_status_read_since_int &= ~(1 << param);
		update_interrupts();
	}

	const bool enable_output = m_control_reg[param] & COUNT_OUT_EN;
	switch (m_mode[param])
	{
		case 0:
		case 2:
			m_output[param] = !m_output[param];
			if (enable_output)
			{
				m_out_cb[param](m_output[param]);
			}
			else
			{
				m_out_cb[param](0);
			}
			LOGMASKED(LOG_TIMEOUTS, "%6.6f: **ptm6840 t%d output %d **\n", machine().time().as_double(), param + 1, m_output[param]);
			break;

		case 4:
		case 6:
			m_output[param] = !m_output[param];
			LOGMASKED(LOG_TIMEOUTS, "**ptm6840 t%d output %d **\n", param + 1, m_output[param]);

			if (!m_single_fired[param])
			{
				if (enable_output)
				{
					m_out_cb[param](m_output[param]);
				}

				if (!m_output[param])
				{
					// Don't allow output to change until reinitialization
					m_single_fired[param] = true;
				}
			}
			else
			{
				m_out_cb[param](0);
			}
			break;
	}

	m_enabled[param] = false;
	reload_counter(param);
}


//-------------------------------------------------
//  set_gate - set gate status (0 or 1)
//-------------------------------------------------

void ptm6840_device::set_gate(int idx, int state)
{
	const bool one_shot_mode = m_mode[idx] == 4 || m_mode[idx] == 6;
	if (state == 0 && m_gate[idx])
	{
		if (!(m_control_reg[0] & RESET_TIMERS))
		{
			m_single_fired[idx] = false;
			m_output[idx] = false;
			reload_counter(idx);
		}
		if (!m_disable_time[idx].is_never() && ((m_control_reg[idx] & INTERNAL_CLK_EN) || m_external_clock[idx] != 0.0))
		{
			m_timer[idx]->adjust(m_disable_time[idx], idx);
			m_disable_time[idx] = attotime::never;
		}
	}
	else if (state == 1 && !m_gate[idx] && !one_shot_mode) // Gate disable is ignored in one-shot mode
	{
		m_disable_time[idx] = m_timer[idx]->remaining();
		m_timer[idx]->adjust(attotime::never);
	}
	m_gate[idx] = state;
}


//-------------------------------------------------
//  set_clock - set clock status (0 or 1)
//-------------------------------------------------

void ptm6840_device::set_clock(int idx, int state)
{
	if (m_clk[idx] == state)
	{
		return;
	}

	const bool old_clk = m_clk[idx];
	m_clk[idx] = state;
	const bool rising_edge = !old_clk && state;
	if (rising_edge)
	{
		return;
	}

	const bool use_external_clk = !(m_control_reg[idx] & INTERNAL_CLK_EN);
	const bool timer_running = !(m_control_reg[0] & RESET_TIMERS);
	const bool one_shot_mode = m_mode[idx] == 4 || m_mode[idx] == 6;
	const bool gated = !one_shot_mode && m_gate[idx];

	// Don't allow ticking if timers are held in reset, internally-clocked, or gated
	if (use_external_clk && timer_running && !gated)
	{
		if (idx == 2)
		{
			m_t3_scaler++;
			if (m_t3_scaler >= m_t3_divisor)
			{
				deduct_from_counter(idx);
				m_t3_scaler = 0;
			}
		}
		else
		{
			deduct_from_counter(idx);
		}
	}
}


//-------------------------------------------------
//  update_expiration_for_clock_source
//-------------------------------------------------

void ptm6840_device::update_expiration_for_clock_source(int idx, bool changed_to_external, double new_external_clock)
{
	if (!(m_control_reg[0] & RESET_TIMERS))
	{
		double divisor = idx == 2 ? m_t3_divisor : 1.0;
		double clk = (m_control_reg[idx] & INTERNAL_CLK_EN ? static_cast<double>(clock()) : new_external_clock) / divisor;

		// First, figure out how much time was remaining on the counter
		if (changed_to_external)
		{
			m_control_reg[idx] |= INTERNAL_CLK_EN;
		}

		int updated_counter = compute_counter(idx);

		if (changed_to_external)
		{
			m_control_reg[idx] &= ~INTERNAL_CLK_EN;
		}

		if (clk == 0.0)
		{
			// If we're externally clocked with no fixed incoming clock

			// Adjust for dual-byte mode if we're not in the last countdown
			if ((m_control_reg[idx] & COUNT_MODE_8BIT) && !m_output[idx])
			{
				const uint16_t latch_lsb = m_latch[idx] & 0xff;
				const uint16_t latch_msb = m_latch[idx] >> 8;
				const uint8_t count_lsb = updated_counter % latch_msb;
				const uint8_t count_msb = (updated_counter - count_lsb) / (latch_lsb + 1);
				m_counter[idx] = (count_msb << 8) | count_lsb;
			}
			else
			{
				m_counter[idx] = updated_counter;
			}

			m_enabled[idx] = false;
			m_timer[idx]->adjust(attotime::never);
			return;
		}
		else
		{
			// If we're externally clocked with a valid incoming clock OR we're internally-clocked
			attotime duration = attotime::from_hz(clk) * updated_counter;

			m_enabled[idx] = true;

			const bool one_shot_mode = m_mode[idx] == 4 || m_mode[idx] == 6;
			const bool gated = !one_shot_mode && m_gate[idx];
			if (gated)
			{
				m_timer[idx]->adjust(attotime::never);
				m_disable_time[idx] = duration;
			}
			else
			{
				m_timer[idx]->adjust(duration, idx);
			}
		}
	}
}



//-------------------------------------------------
//  set_ext_clock - set external clock frequency
//-------------------------------------------------

void ptm6840_device::set_ext_clock(int idx, double clk)
{
	if (m_external_clock[idx] == clk)
		return;

	if (!(m_control_reg[idx] & INTERNAL_CLK_EN))
	{
		update_expiration_for_clock_source(idx, false, clk);
	}

	m_external_clock[idx] = clk;
}
