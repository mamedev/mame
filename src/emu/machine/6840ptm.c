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
#include "devhelpr.h"


#define PTMVERBOSE 0
#define PLOG(x) do { if (PTMVERBOSE) logerror x; } while (0)

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

GENERIC_DEVICE_CONFIG_SETUP(ptm6840, "6840 PTM")

const device_type PTM6840 = ptm6840_device_config::static_alloc_device_config;

//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void ptm6840_device_config::device_config_complete()
{
	// inherit a copy of the static data
	const ptm6840_interface *intf = reinterpret_cast<const ptm6840_interface *>(static_config());
	if (intf != NULL)
	{
		*static_cast<ptm6840_interface *>(this) = *intf;
	}

	// or initialize to defaults if none provided
	else
	{
		m_internal_clock = 0.0;
		m_external_clock[0] = 0.0;
		m_external_clock[1] = 0.0;
		m_external_clock[2] = 0.0;
		memset(&m_irq_func, 0, sizeof(m_irq_func));
    	memset(&m_out_func[0], 0, sizeof(m_out_func[0]));
    	memset(&m_out_func[1], 0, sizeof(m_out_func[1]));
    	memset(&m_out_func[2], 0, sizeof(m_out_func[2]));
	}
}


//-------------------------------------------------
//  ptm6840_device - constructor
//-------------------------------------------------

ptm6840_device::ptm6840_device(running_machine &_machine, const ptm6840_device_config &config)
    : device_t(_machine, config),
      m_config(config)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void ptm6840_device::device_start()
{
	m_internal_clock = m_config.m_internal_clock;
	/* resolve callbacks */
	for (int i = 0; i < 3; i++)
	{
		devcb_resolve_write8(&m_out_func[i], &m_config.m_out_func[i], this);
	}

	for (int i = 0; i < 3; i++)
	{
		if ( m_config.m_external_clock[i] )
		{
			m_external_clock[i] = m_config.m_external_clock[i];
		}
		else
		{
			m_external_clock[i] = 1;
		}
	}


	m_timer[0] = timer_alloc(&m_machine, ptm6840_timer1_cb, (void *)this);
	m_timer[1] = timer_alloc(&m_machine, ptm6840_timer2_cb, (void *)this);
	m_timer[2] = timer_alloc(&m_machine, ptm6840_timer3_cb, (void *)this);

	for (int i = 0; i < 3; i++)
	{
		timer_enable(m_timer[i], FALSE);
	}

	devcb_resolve_write_line(&m_irq_func, &m_config.m_irq_func, this);

	/* register for state saving */
	state_save_register_device_item(this, 0, m_lsb_buffer);
	state_save_register_device_item(this, 0, m_msb_buffer);
	state_save_register_device_item(this, 0, m_status_read_since_int);
	state_save_register_device_item(this, 0, m_status_reg);
	state_save_register_device_item(this, 0, m_t3_divisor);
	state_save_register_device_item(this, 0, m_t3_scaler);
	state_save_register_device_item(this, 0, m_internal_clock);
	state_save_register_device_item(this, 0, m_IRQ);

	state_save_register_device_item_array(this, 0, m_control_reg);
	state_save_register_device_item_array(this, 0, m_output);
	state_save_register_device_item_array(this, 0, m_gate);
	state_save_register_device_item_array(this, 0, m_clk);
	state_save_register_device_item_array(this, 0, m_mode);
	state_save_register_device_item_array(this, 0, m_fired);
	state_save_register_device_item_array(this, 0, m_enabled);
	state_save_register_device_item_array(this, 0, m_external_clock);
	state_save_register_device_item_array(this, 0, m_counter);
	state_save_register_device_item_array(this, 0, m_latch);
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void ptm6840_device::device_reset()
{
	m_control_reg[2]		 = 0;
	m_control_reg[1]		 = 0;
	m_control_reg[0]		 = 1;
	m_status_reg			 = 0;
	m_t3_divisor			 = 1;
	m_status_read_since_int = 0;
	m_IRQ                   = 0;
	for (int i = 0; i < 3; i++)
	{
		m_counter[i] = 0xffff;
		m_latch[i]   = 0xffff;
		m_output[i]  = 0;
		m_fired[i]   = 0;
	}
}



/*-------------------------------------------------
    ptm6840_get_status - Get enabled status
-------------------------------------------------*/

int ptm6840_get_status( device_t *device, int clock )
{
	return downcast<ptm6840_device*>(device)->ptm6840_get_status(clock);
}

int ptm6840_device::ptm6840_get_status( int clock )
{
	return m_enabled[clock - 1];
}



/*-------------------------------------------------
    ptm6840_get_irq - Get IRQ state
-------------------------------------------------*/

int ptm6840_get_irq( device_t *device )
{
	return downcast<ptm6840_device*>(device)->ptm6840_get_irq();
}

int ptm6840_device::ptm6840_get_irq()
{
	return m_IRQ;
}



/*-------------------------------------------------
    subtract_from_counter - Subtract from Counter
-------------------------------------------------*/

void ptm6840_device::subtract_from_counter(int counter, int count)
{
	double clock;

	/* Determine the clock frequency for this timer */
	if (m_control_reg[counter] & 0x02)
	{
		clock = m_internal_clock;
	}
	else
	{
		clock = m_external_clock[counter];
	}

	/* Dual-byte mode */
	if (m_control_reg[counter] & 0x04)
	{
		int lsb = m_counter[counter] & 0xff;
		int msb = m_counter[counter] >> 8;

		/* Count the clocks */
		lsb -= count;

		/* Loop while we're less than zero */
		while (lsb < 0)
		{
			/* Borrow from the MSB */
			lsb += (m_latch[counter] & 0xff) + 1;
			msb--;

			/* If MSB goes less than zero, we've expired */
			if (msb < 0)
			{
				ptm6840_timeout(counter);
				msb = (m_latch[counter] >> 8) + 1;
			}
		}

		/* Store the result */
		m_counter[counter] = (msb << 8) | lsb;
	}

	/* Word mode */
	else
	{
		int word = m_counter[counter];

		/* Count the clocks */
		word -= count;

		/* loop while we're less than zero */
		while (word < 0)
		{
			/* Borrow from the MSB */
			word += m_latch[counter] + 1;

			/* We've expired */
			ptm6840_timeout(counter);
		}

		/* Store the result */
		m_counter[counter] = word;
	}

	if (m_enabled[counter])
	{
		attotime duration = attotime_mul(ATTOTIME_IN_HZ(clock), m_counter[counter]);

		if (counter == 2)
		{
			duration = attotime_mul(duration, m_t3_divisor);
		}

		timer_adjust_oneshot(m_timer[counter], duration, 0);
	}
}



/*-------------------------------------------------
    ptm_tick
-------------------------------------------------*/

void ptm6840_device::ptm_tick(int counter, int count)
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



/*-------------------------------------------------
    update_interrupts - Update Internal Interrupts
-------------------------------------------------*/

void update_interrupts( device_t *device )
{
	downcast<ptm6840_device*>(device)->update_interrupts();
}

void ptm6840_device::update_interrupts()
{
	int new_state = ((m_status_reg & 0x01) && (m_control_reg[0] & 0x40)) ||
					((m_status_reg & 0x02) && (m_control_reg[1] & 0x40)) ||
					((m_status_reg & 0x04) && (m_control_reg[2] & 0x40));

	if (new_state != m_IRQ)
	{
		m_IRQ = new_state;

		if (m_IRQ)
		{
			m_status_reg |= 0x80;
		}
		else
		{
			m_status_reg &= ~0x80;
		}

		devcb_call_write_line(&m_irq_func, m_IRQ);
	}
}



/*-------------------------------------------------
    compute_counter - Compute Counter
-------------------------------------------------*/

UINT16 ptm6840_device::compute_counter( int counter )
{
	double clock;

	/* If there's no timer, return the count */
	if (!m_enabled[counter])
	{
		PLOG(("MC6840 #%s: read counter(%d): %d\n", tag(), counter, m_counter[counter]));
		return m_counter[counter];
	}

	/* determine the clock frequency for this timer */
	if (m_control_reg[counter] & 0x02)
	{
		clock = m_internal_clock;
		PLOG(("MC6840 #%s: %d internal clock freq %f \n", tag(), counter, clock));
	}
	else
	{
		clock = m_external_clock[counter];
		PLOG(("MC6840 #%s: %d external clock freq %f \n", tag(), counter, clock));
	}
	/* See how many are left */
	int remaining = attotime_to_double(attotime_mul(timer_timeleft(m_timer[counter]), clock));

	/* Adjust the count for dual byte mode */
	if (m_control_reg[counter] & 0x04)
	{
		int divisor = (m_counter[counter] & 0xff) + 1;
		int msb = remaining / divisor;
		int lsb = remaining % divisor;
		remaining = (msb << 8) | lsb;
	}
	PLOG(("MC6840 #%s: read counter(%d): %d\n", tag(), counter, remaining));
	return remaining;
}



/*-------------------------------------------------
    reload_count - Reload Counter
-------------------------------------------------*/

void ptm6840_device::reload_count(int idx)
{
	double clock;

	/* Copy the latched value in */
	m_counter[idx] = m_latch[idx];

	/* Determine the clock frequency for this timer */
	if (m_control_reg[idx] & 0x02)
	{
		clock = m_internal_clock;
		PLOG(("MC6840 #%s: %d internal clock freq %f \n", tag(), idx, clock));
	}
	else
	{
		clock = m_external_clock[idx];
		PLOG(("MC6840 #%s: %d external clock freq %f \n", tag(), idx, clock));
	}

	/* Determine the number of clock periods before we expire */
	int count = m_counter[idx];
	if (m_control_reg[idx] & 0x04)
	{
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
		if (m_out_func[idx].write != NULL)
		{
			devcb_call_write8(&m_out_func[idx], 0, m_output[idx]);
		}
	}

	/* Set the timer */
	PLOG(("MC6840 #%s: reload_count(%d): clock = %f  count = %d\n", tag(), idx, clock, count));

	attotime duration = attotime_mul(ATTOTIME_IN_HZ(clock), count);
	if (idx == 2)
	{
		duration = attotime_mul(duration, m_t3_divisor);
	}

	PLOG(("MC6840 #%s: reload_count(%d): output = %lf\n", tag(), idx, attotime_to_double(duration)));

#if 0
	if (!(m_control_reg[idx] & 0x02))
	{
		if (!m_external_clock[idx])
		{
			m_enabled[idx] = 0;
			timer_enable(m_timer[idx],FALSE);
		}
	}
	else
#endif
	{
		m_enabled[idx] = 1;
		timer_adjust_oneshot(m_timer[idx], duration, 0);
		timer_enable(m_timer[idx], TRUE);
	}
}



/*-------------------------------------------------
    ptm6840_read - Read Timer
-------------------------------------------------*/

READ8_DEVICE_HANDLER_TRAMPOLINE(ptm6840, ptm6840_read)
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
			PLOG(("%s: MC6840 #%s: Status read = %04X\n", cpuexec_describe_context(&m_machine), tag(), m_status_reg));
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

			/* Clear the interrupt if the status has been read */
			if (m_status_read_since_int & (1 << idx))
			{
				m_status_reg &= ~(1 << idx);
				update_interrupts();
			}

			m_lsb_buffer = result & 0xff;

			PLOG(("%s: MC6840 #%s: Counter %d read = %04X\n", cpuexec_describe_context(&m_machine), tag(), idx, result >> 8));
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


/*-------------------------------------------------
    ptm6840_write - Write Timer
-------------------------------------------------*/

WRITE8_DEVICE_HANDLER_TRAMPOLINE(ptm6840, ptm6840_write)
{
	switch ( offset )
	{
		case PTM_6840_CTRL1:
		case PTM_6840_CTRL2:
		{
			int idx = (offset == 1) ? 1 : (m_control_reg[1] & 0x01) ? 0 : 2;
			UINT8 diffs = data ^ m_control_reg[idx];
			m_t3_divisor = (m_control_reg[2] & 0x01) ? 8 : 1;
			m_mode[idx] = (data >> 3) & 0x07;
			m_control_reg[idx] = data;

			PLOG(("MC6840 #%s : Control register %d selected\n", tag(), idx));
			PLOG(("operation mode   = %s\n", opmode[ m_mode[idx] ]));
			PLOG(("value            = %04X\n", m_control_reg[idx]));
			PLOG(("t3divisor        = %d\n", m_t3_divisor));

			if (!(m_control_reg[idx] & 0x80 ))
			{
				/* Output cleared */
				devcb_call_write8(&m_out_func[idx], 0, 0);
			}
			/* Reset? */
			if (idx == 0 && (diffs & 0x01))
			{
				/* Holding reset down */
				if (data & 0x01)
				{
					PLOG(("MC6840 #%s : Timer reset\n", tag()));
					for (int i = 0; i < 3; i++)
					{
						timer_enable(m_timer[i], FALSE);
						m_enabled[i] = 0;
					}
				}
				/* Releasing reset */
				else
				{
					for (int i = 0; i < 3; i++)
					{
						reload_count(i);
					}
				}

				m_status_reg = 0;
				update_interrupts();

				/* Changing the clock source? (e.g. Zwackery) */
				if (diffs & 0x02)
				{
					reload_count(idx);
				}
			}
			break;
		}

		case PTM_6840_MSBBUF1:
		case PTM_6840_MSBBUF2:
		case PTM_6840_MSBBUF3:
		{
			PLOG(("MC6840 #%s msbbuf%d = %02X\n", tag(), offset / 2, data));
			m_msb_buffer = data;
			break;
		}

		case PTM_6840_LSB1:
		case PTM_6840_LSB2:
		case PTM_6840_LSB3:
		{
			int idx = (offset - 3) / 2;
			m_latch[idx] = (m_msb_buffer << 8) | (data & 0xff);

			/* Clear the interrupt */
			m_status_reg &= ~(1 << idx);
			update_interrupts();

			/* Reload the count if in an appropriate mode */
			if (!(m_control_reg[idx] & 0x10))
			{
				reload_count(idx);
			}

			PLOG(("%s:MC6840 #%s: Counter %d latch = %04X\n", cpuexec_describe_context(&m_machine), tag(), idx, m_latch[idx]));
			break;
		}
	}
}


/*-------------------------------------------------
    ptm6840_timeout - Called if timer is mature
-------------------------------------------------*/

void ptm6840_device::ptm6840_timeout(int idx)
{
	PLOG(("**ptm6840 %s t%d timeout**\n", tag(), idx + 1));

	/* Set the interrupt flag */
	m_status_reg |= (1 << idx);
	m_status_read_since_int &= ~(1 << idx);
	update_interrupts();

	if ( m_control_reg[idx] & 0x80 )
	{
		if ((m_mode[idx] == 0)||(m_mode[idx] == 2))
		{
			m_output[idx] = m_output[idx] ? 0 : 1;
			PLOG(("**ptm6840 %s t%d output %d **\n", tag(), idx + 1, m_output[idx]));

			devcb_call_write8(&m_out_func[idx], 0, m_output[idx]);
		}
		if ((m_mode[idx] == 4)||(m_mode[idx] == 6))
		{
			if (!m_fired[idx])
			{
				m_output[idx] = 1;
				PLOG(("**ptm6840 %s t%d output %d **\n", tag(), idx + 1, m_output[idx]));

				devcb_call_write8(&m_out_func[idx], 0, m_output[idx]);

				/* No changes in output until reinit */
				m_fired[idx] = 1;

				m_status_reg |= (1 << idx);
				m_status_read_since_int &= ~(1 << idx);
				update_interrupts();
			}
		}
	}
	m_enabled[idx]= 0;
	reload_count(idx);
}


/*-------------------------------------------------
    TIMER_CALLBACKs for Timer 1, 2 & 3
-------------------------------------------------*/

TIMER_CALLBACK( ptm6840_device::ptm6840_timer1_cb )
{
	reinterpret_cast<ptm6840_device *>(ptr)->ptm6840_timeout(0);
}

TIMER_CALLBACK( ptm6840_device::ptm6840_timer2_cb )
{
	reinterpret_cast<ptm6840_device *>(ptr)->ptm6840_timeout(1);
}

TIMER_CALLBACK( ptm6840_device::ptm6840_timer3_cb )
{
	reinterpret_cast<ptm6840_device *>(ptr)->ptm6840_timeout(2);
}


/*-------------------------------------------------
    ptm6840_set_gate - set gate status (0 or 1)
-------------------------------------------------*/

void ptm6840_device::ptm6840_set_gate(int state, int idx)
{
	if ( (m_mode[idx] == 0) || (m_mode[idx] == 2) || (m_mode[0] == 4) || (m_mode[idx] == 6) )
	{
		if (state == 0 && m_gate[idx])
		{
			reload_count(idx);
		}
	}
	m_gate[idx] = state;
}


/*-------------------------------------------------
    WRITE8_DEVICE_HANDLERs for Gate 1, 2 & 3
-------------------------------------------------*/

WRITE8_DEVICE_HANDLER( ptm6840_set_g1 )
{
	downcast<ptm6840_device*>(device)->ptm6840_set_gate(data, 0);
}

WRITE8_DEVICE_HANDLER( ptm6840_set_g2 )
{
	downcast<ptm6840_device*>(device)->ptm6840_set_gate(data, 1);
}

WRITE8_DEVICE_HANDLER( ptm6840_set_g3 )
{
	downcast<ptm6840_device*>(device)->ptm6840_set_gate(data, 2);
}


/*-------------------------------------------------
    ptm6840_set_clock - set clock status (0 or 1)
-------------------------------------------------*/

void ptm6840_device::ptm6840_set_clock(int state, int idx)
{
	m_clk[idx] = state;

	if (!(m_control_reg[idx] & 0x02))
	{
		if (state)
		{
			ptm_tick(idx, 1);
		}
	}
}


/*-------------------------------------------------
    WRITE8_DEVICE_HANDLERs for Clock 1, 2 & 3
-------------------------------------------------*/

WRITE8_DEVICE_HANDLER( ptm6840_set_c1 )
{
	downcast<ptm6840_device*>(device)->ptm6840_set_clock(data, 0);
}

WRITE8_DEVICE_HANDLER( ptm6840_set_c2 )
{
	downcast<ptm6840_device*>(device)->ptm6840_set_clock(data, 1);
}

WRITE8_DEVICE_HANDLER( ptm6840_set_c3 )
{
	downcast<ptm6840_device*>(device)->ptm6840_set_clock(data, 2);
}


/*-------------------------------------------------
    ptm6840_get_count - get count value
-------------------------------------------------*/

UINT16 ptm6840_get_count(device_t *device, int counter)
{
	return downcast<ptm6840_device*>(device)->ptm6840_get_count(counter);
}

UINT16 ptm6840_device::ptm6840_get_count(int counter)
{
	return compute_counter(counter);
}


/*------------------------------------------------------------
    ptm6840_set_ext_clock - set external clock frequency
------------------------------------------------------------*/

void ptm6840_set_ext_clock(device_t *device, int counter, double clock)
{
	downcast<ptm6840_device*>(device)->ptm6840_set_ext_clock(counter, clock);
}

void ptm6840_device::ptm6840_set_ext_clock(int counter, double clock)
{
	m_external_clock[counter] = clock;

	if (!(m_control_reg[counter] & 0x02))
	{
		if (!m_external_clock[counter])
		{
			m_enabled[counter] = 0;
			timer_enable(m_timer[counter], FALSE);
		}
	}
	else
	{
		int count;
		attotime duration;

		/* Determine the number of clock periods before we expire */
		count = m_counter[counter];

		if (m_control_reg[counter] & 0x04)
		{
			count = ((count >> 8) + 1) * ((count & 0xff) + 1);
		}
		else
		{
			count = count + 1;
		}

		duration = attotime_mul(ATTOTIME_IN_HZ(clock), count);

		if (counter == 2)
		{
			duration = attotime_mul(duration, m_t3_divisor);
		}

		m_enabled[counter] = 1;
		timer_adjust_oneshot(m_timer[counter], duration, 0);
		timer_enable(m_timer[counter], TRUE);
	}
}


/*------------------------------------------------------------
    ptm6840_get_ext_clock - get external clock frequency
------------------------------------------------------------*/

int ptm6840_get_ext_clock( device_t *device, int counter )
{
	return downcast<ptm6840_device*>(device)->ptm6840_get_ext_clock(counter);
}

int ptm6840_device::ptm6840_get_ext_clock(int counter)
{
	return m_external_clock[counter];
}
