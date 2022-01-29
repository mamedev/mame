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

	m_timer[0] = timer_alloc(0);
	m_timer[1] = timer_alloc(1);
	m_timer[2] = timer_alloc(2);

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
	save_item(NAME(m_fired));
	save_item(NAME(m_enabled));
	save_item(NAME(m_external_clock));
	save_item(NAME(m_counter));
	save_item(NAME(m_latch));
	save_item(NAME(m_hightime));
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
	m_hightime[0]           = false;
	m_hightime[1]           = false;
	m_hightime[2]           = false;

	for (int i = 0; i < 3; i++)
	{
		m_counter[i] = 0xffff;
		m_latch[i]   = 0xffff;
		m_output[i]  = 0;
		m_fired[i]   = 0;
		m_enabled[i] = 0;
		m_mode[i] = 0;
	}
}

void ptm6840_device::device_resolve_objects()
{
	for (int i = 0; i < 3; i++)
		m_gate[i]  = 0;
}

//-------------------------------------------------
//  device_timer - handle timer callbacks
//-------------------------------------------------

void ptm6840_device::device_timer(emu_timer &timer, device_timer_id id, int param)
{
	timeout(id);
}


//-------------------------------------------------
//  subtract_from_counter - Subtract from Counter
//-------------------------------------------------

void ptm6840_device::subtract_from_counter(int counter, int count)
{
	// Determine the clock frequency for this timer
	double clk = m_control_reg[counter] & INTERNAL_CLK_EN ? static_cast<double>(clock()) : m_external_clock[counter];

	// Dual-byte mode
	if (m_control_reg[counter] & COUNT_MODE_8BIT)
	{
		int lsb = m_counter[counter] & 0xff;
		int msb = m_counter[counter] >> 8;

		// Count the clocks
		lsb -= count;

		// Loop while we're less than zero
		while (lsb < 0)
		{
			// Borrow from the MSB
			lsb += (m_latch[counter] & 0xff) + 1;
			msb--;

			// If MSB goes less than zero, we've expired
			if ((msb == 0 && !m_hightime[counter]) || (msb < 0 && m_hightime[counter]))
			{
				timeout(counter);
				msb = (m_latch[counter] >> 8) + 1;
			}
		}

		// Store the result
		m_counter[counter] = (msb << 8) | lsb;
	}

	// Word mode
	else
	{
		int word = m_counter[counter];

		// Count the clocks
		word -= count;

		// loop while we're less than zero
		while (word < 0)
		{
			// Borrow from the MSB
			word += m_latch[counter] + 1;

			// We've expired
			timeout(counter);
		}

		// Store the result
		m_counter[counter] = word;
	}

	if (m_enabled[counter])
	{
		int clks = m_counter[counter];
		if (m_control_reg[counter] & COUNT_MODE_8BIT)
		{
			/* In dual 8 bit mode, let the counter fire when MSB == 0 */
			m_hightime[counter] = !(clks & 0xff00);
			clks &= 0xff00;
		}

		attotime duration = attotime::from_hz(clk) * clks;
		if (counter == 2)
		{
			duration *= m_t3_divisor;
		}
		m_timer[counter]->adjust(duration);
	}
}



//-------------------------------------------------
//  tick
//-------------------------------------------------

void ptm6840_device::tick(int counter, int count)
{
	if (counter == 2)
	{
		m_t3_scaler += count;

		if ( m_t3_scaler > m_t3_divisor - 1)
		{
			subtract_from_counter(counter, 1);
			m_t3_scaler = 0;
		}
	}
	else
	{
		subtract_from_counter(counter, count);
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

uint16_t ptm6840_device::compute_counter( int counter ) const
{
	double clk;

	// If there's no timer, return the count
	if (!m_enabled[counter])
	{
		LOGMASKED(LOG_COUNTERS, "Timer #%d read counter: %d\n", counter + 1, m_counter[counter]);
		return m_counter[counter];
	}

	// determine the clock frequency for this timer
	if (m_control_reg[counter] & INTERNAL_CLK_EN)
	{
		clk = static_cast<double>(clock());
	}
	else
	{
		clk = m_external_clock[counter];
	}

	if (counter == 2)
	{
		clk /= m_t3_divisor;
	}
	LOGMASKED(LOG_COUNTERS, "Timer #%d %s clock freq %f \n", counter + 1, (m_control_reg[counter] & INTERNAL_CLK_EN) ? "internal" : "external", clk);

	// See how many are left
	int remaining = (m_timer[counter]->remaining() * clk).as_double();

	// Adjust the count for dual byte mode
	if (m_control_reg[counter] & COUNT_MODE_8BIT)
	{
		int divisor = (m_counter[counter] & 0xff) + 1;
		int msb = remaining / divisor;
		int lsb = remaining % divisor;
		remaining = (msb << 8) | lsb;
	}

	LOGMASKED(LOG_COUNTERS, "Timer #%d read counter: %d\n", counter + 1, remaining);
	return remaining;
}



//-------------------------------------------------
//  reload_count - Reload Counter
//-------------------------------------------------

void ptm6840_device::reload_count(int idx)
{
	double clk;

	// Copy the latched value in
	m_counter[idx] = m_latch[idx];

	// If reset is held, don't start counting
	if (m_control_reg[0] & RESET_TIMERS)
		return;

	// Determine the clock frequency for this timer
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
	int count = m_counter[idx];
	if (m_control_reg[idx] & COUNT_MODE_8BIT)
	{
		if (m_hightime[idx])
			count = 0xff;
		else
			count = ((count >> 8) + 1) * ((count & 0xff) + 1);

	}
	else
	{
		count = count + 1;
	}

	m_fired[idx] = 0;

	if ((m_mode[idx] == 4) || (m_mode[idx] == 6))
	{
		m_output[idx] = 1;
		m_out_cb[idx](m_output[idx]);
	}

	// Set the timer
	LOGMASKED(LOG_COUNTERS, "Timer #%d reload_count: clock = %f  count = %d\n", idx + 1, clk, count);

	if (clk == 0.0)
	{
		m_enabled[idx] = 0;
		m_timer[idx]->enable(false);
	}
	else
	{
		attotime duration = attotime::from_hz(clk) * count;

		if (idx == 2)
		{
			duration *= m_t3_divisor;
		}

		LOGMASKED(LOG_COUNTERS, "Timer #%d reload_count: output = %f\n", idx + 1, duration.as_double());

		m_enabled[idx] = 1;
		m_timer[idx]->adjust(duration);
		m_timer[idx]->enable(true);
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
		{
			val = 0;
			break;
		}

		case PTM_6840_STATUS:
		{
			LOGMASKED(LOG_STATUS, "%s: Status read = %04X\n", machine().describe_context(), m_status_reg);
			m_status_read_since_int |= m_status_reg & 0x07;
			val = m_status_reg;
			break;
		}

		case PTM_6840_MSBBUF1:
		case PTM_6840_MSBBUF2:
		case PTM_6840_MSBBUF3:
		{
			int idx = (offset - 2) / 2;
			int result = compute_counter(idx);

			// Clear the interrupt if the status has been read
			if (m_status_read_since_int & (1 << idx))
			{
				m_status_reg &= ~(1 << idx);
				update_interrupts();
			}

			m_lsb_buffer = result & 0xff;

			LOGMASKED(LOG_COUNTERS, "%s: Counter %d read = %04X\n", machine().describe_context(), idx + 1, result >> 8);
			val = result >> 8;
			break;
		}

		case PTM_6840_LSB1:
		case PTM_6840_LSB2:
		case PTM_6840_LSB3:
		{
			val = m_lsb_buffer;
			break;
		}

		default:
		{
			val = 0;
			break;
		}

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
			LOGMASKED(LOG_CONTROL, "value            = %04X\n", m_control_reg[idx]);
			LOGMASKED(LOG_CONTROL, "t3divisor        = %d\n", m_t3_divisor);

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
					LOGMASKED(LOG_RESETS, "Timer reset\n");
					for (int i = 0; i < 3; i++)
					{
						m_timer[i]->enable(false);
						m_enabled[i] = 0;
						m_hightime[idx] = false;
					}
				}
				// Releasing reset
				else
				{
					for (int i = 0; i < 3; i++)
					{
						m_hightime[idx] = false;
						reload_count(i);
					}
				}

				m_status_reg = 0;
				update_interrupts();
			}

			// Changing the clock source? (e.g. Zwackery)
			if (diffs & INTERNAL_CLK_EN)
			{
				m_hightime[idx] = false;
				reload_count(idx);
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
				m_hightime[idx] = false;
				reload_count(idx);
			}

			LOGMASKED(LOG_COUNTERS, "%s: Counter #%d latch = %04X\n", machine().describe_context(), idx + 1, m_latch[idx]);
			break;
		}
	}
}


//-------------------------------------------------
//  timeout - Called if timer is mature
//-------------------------------------------------

void ptm6840_device::timeout(int idx)
{
	LOGMASKED(LOG_TIMEOUTS, "**ptm6840 t%d timeout**\n", idx + 1);

	// Set the interrupt flag
	m_status_reg |= (1 << idx);
	m_status_read_since_int &= ~(1 << idx);
	update_interrupts();

	if (m_control_reg[idx] & COUNT_OUT_EN)
	{
		switch (m_mode[idx])
		{
			case 0:
			case 2:

				if (m_control_reg[idx] & COUNT_MODE_8BIT)
				{
					m_hightime[idx] = !m_hightime[idx];
					m_output[idx] = m_hightime[idx];
					m_out_cb[idx](m_output[idx]);
				}
				else
				{
					m_output[idx] = m_output[idx] ^ 1;
					m_out_cb[idx](m_output[idx]);
				}
				LOGMASKED(LOG_TIMEOUTS, "%6.6f: **ptm6840 t%d output %d **\n", machine().time().as_double(), idx + 1, m_output[idx]);
				break;

			case 4:
			case 6:
				if (!m_fired[idx])
				{
					m_output[idx] = 1;
					LOGMASKED(LOG_TIMEOUTS, "**ptm6840 t%d output %d **\n", idx + 1, m_output[idx]);

					m_out_cb[idx](m_output[idx]);

					// No changes in output until reinit
					m_fired[idx] = 1;

					m_status_reg |= (1 << idx);
					m_status_read_since_int &= ~(1 << idx);
					update_interrupts();
				}
				break;
		}
	}
	m_enabled[idx]= 0;
	reload_count(idx);
}


//-------------------------------------------------
//  set_gate - set gate status (0 or 1)
//-------------------------------------------------

void ptm6840_device::set_gate(int idx, int state)
{
	if ((m_mode[idx] & 1) == 0)
	{
		if (state == 0 && m_gate[idx])
		{
			m_hightime[idx] = false;
			reload_count(idx);
		}
	}
	m_gate[idx] = state;
}


//-------------------------------------------------
//  set_clock - set clock status (0 or 1)
//-------------------------------------------------

void ptm6840_device::set_clock(int idx, int state)
{
	m_clk[idx] = state;

	if (!(m_control_reg[idx] & INTERNAL_CLK_EN))
	{
		if (state)
		{
			tick(idx, 1);
		}
	}
}


//-------------------------------------------------
//  set_ext_clock - set external clock frequency
//-------------------------------------------------

void ptm6840_device::set_ext_clock(int counter, double clock)
{
	m_external_clock[counter] = clock;

	if (!(m_control_reg[counter] & INTERNAL_CLK_EN))
	{
		if (!m_external_clock[counter])
		{
			m_enabled[counter] = 0;
			m_timer[counter]->enable(false);
		}
	}
	else
	{
		int count;
		attotime duration;

		// Determine the number of clock periods before we expire
		count = m_counter[counter];

		if (m_control_reg[counter] & COUNT_MODE_8BIT)
		{
			count = ((count >> 8) + 1) * ((count & 0xff) + 1);
		}
		else
		{
			count = count + 1;
		}

		duration = attotime::from_hz(clock) * count;

		if (counter == 2)
		{
			duration *= m_t3_divisor;
		}

		m_enabled[counter] = 1;
		m_timer[counter]->adjust(duration);
		m_timer[counter]->enable(true);
	}
}
