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

#include "driver.h"
#include "6840ptm.h"


#define PTMVERBOSE 0
#define PLOG(x) do { if (PTMVERBOSE) logerror x; } while (0)

/***************************************************************************
    PARAMETERS / PROTOTYPES
***************************************************************************/

enum
{
	PTM_6840_CTRL1   = 0,
	PTM_6840_CTRL2   = 1,
	PTM_6840_STATUS  = 1,
	PTM_6840_MSBBUF1 = 2,
	PTM_6840_LSB1	 = 3,
	PTM_6840_MSBBUF2 = 4,
	PTM_6840_LSB2    = 5,
	PTM_6840_MSBBUF3 = 6,
	PTM_6840_LSB3    = 7,
};

static const char *const opmode[] =
{
	"000 continous mode",
	"001 freq comparison mode",
	"010 continous mode",
	"011 pulse width comparison mode",
	"100 single shot mode",
	"101 freq comparison mode",
	"110 single shot mode",
	"111 pulse width comparison mode"
};

static void ptm6840_timeout(const device_config *device, int idx);

/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

typedef struct _ptm6840_state ptm6840_state;
struct _ptm6840_state
{
	int internal_clock;
	int external_clock[3];
	devcb_resolved_write8 out_func[3];	// function to call when output[idx] changes
	devcb_resolved_write_line irq_func;	// function called if IRQ line changes

	UINT8 control_reg[3];
	UINT8 output[3]; /* Output states */
	UINT8 gate[3];   /* Input gate states */
	UINT8 clock[3];  /* Clock states */
	UINT8 enabled[3];
	UINT8 mode[3];
	UINT8 fired[3];
	UINT8 t3_divisor;
	UINT8 t3_scaler;
	UINT8 IRQ;
	UINT8 status_reg;
	UINT8 status_read_since_int;
	UINT8 lsb_buffer;
	UINT8 msb_buffer;

	/* Each PTM has 3 timers */
	emu_timer *timer[3];

	UINT16 latch[3];
	UINT16 counter[3];
};


/***************************************************************************
    INLINE FUNCTIONS
***************************************************************************/

INLINE ptm6840_state *get_safe_token(const device_config *device)
{
	assert(device != NULL);
	assert(device->token != NULL);
	assert((device->type == PTM6840));
	return (ptm6840_state *)device->token;
}

INLINE const ptm6840_interface *get_interface(const device_config *device)
{
	assert(device != NULL);
	assert((device->type == PTM6840));
	return (const ptm6840_interface *) device->static_config;
}


/***************************************************************************
    IMPLEMENTATION
***************************************************************************/

/*-------------------------------------------------
    ptm6840_get_status - Get enabled status
-------------------------------------------------*/

int ptm6840_get_status( const device_config *device, int clock )
{
	ptm6840_state *ptm6840 = get_safe_token(device);
	return ptm6840->enabled[clock - 1];
}

/*-------------------------------------------------
    ptm6840_get_irq - Get IRQ state
-------------------------------------------------*/

int ptm6840_get_irq( const device_config *device )
{
	ptm6840_state *ptm6840 = get_safe_token(device);
	return ptm6840->IRQ;
}

/*-------------------------------------------------
    subtract_from_counter - Subtract from Counter
-------------------------------------------------*/

static void subtract_from_counter( const device_config *device, int counter, int count )
{
	ptm6840_state *ptm6840 = get_safe_token(device);
	int clock;
	attotime duration;

	/* Determine the clock frequency for this timer */
	if (ptm6840->control_reg[counter] & 0x02)
		clock = ptm6840->internal_clock;
	else
		clock = ptm6840->external_clock[counter];

	/* Dual-byte mode */
	if (ptm6840->control_reg[counter] & 0x04)
	{
		int lsb = ptm6840->counter[counter] & 0xff;
		int msb = ptm6840->counter[counter] >> 8;

		/* Count the clocks */
		lsb -= count;

		/* Loop while we're less than zero */
		while (lsb < 0)
		{
			/* Borrow from the MSB */
			lsb += (ptm6840->latch[counter] & 0xff) + 1;
			msb--;

			/* If MSB goes less than zero, we've expired */
			if (msb < 0)
			{
				ptm6840_timeout(device, counter);
				msb = (ptm6840->latch[counter] >> 8) + 1;
			}
		}

		/* Store the result */
		ptm6840->counter[counter] = (msb << 8) | lsb;
	}

	/* Word mode */
	else
	{
		int word = ptm6840->counter[counter];

		/* Count the clocks */
		word -= count;

		/* loop while we're less than zero */
		while (word < 0)
		{
			/* Borrow from the MSB */
			word += ptm6840->latch[counter] + 1;

			/* We've expired */
			ptm6840_timeout(device, counter);
		}

		/* Store the result */
		ptm6840->counter[counter] = word;
	}

	if (ptm6840->enabled[counter])
	{
		duration = attotime_mul(ATTOTIME_IN_HZ(clock), ptm6840->counter[counter]);

		if (counter == 2)
			duration = attotime_mul(duration, ptm6840->t3_divisor);

		timer_adjust_oneshot(ptm6840->timer[counter], duration, 0);
	}
}

/*-------------------------------------------------
    ptm_tick
-------------------------------------------------*/

static void ptm_tick( const device_config *device, int counter, int count )
{
	ptm6840_state *ptm6840 = get_safe_token(device);

	if (counter == 2)
	{
		ptm6840->t3_scaler += count;

		if ( ptm6840->t3_scaler > ptm6840->t3_divisor - 1)
		{
			subtract_from_counter(device, counter, 1);
			ptm6840->t3_scaler = 0;
		}
	}
	else
	{
		subtract_from_counter(device, counter, count);
	}
}

/*-------------------------------------------------
    update_interrupts - Update Internal Interrupts
-------------------------------------------------*/

INLINE void update_interrupts( const device_config *device )
{
	ptm6840_state *ptm6840 = get_safe_token(device);
	int new_state;

	new_state = ((ptm6840->status_reg & 0x01) && (ptm6840->control_reg[0] & 0x40)) ||
				((ptm6840->status_reg & 0x02) && (ptm6840->control_reg[1] & 0x40)) ||
				((ptm6840->status_reg & 0x04) && (ptm6840->control_reg[2] & 0x40));

//  if (new_state != ptm6840->IRQ)
	{
		ptm6840->IRQ = new_state;

		if (ptm6840->IRQ)
			ptm6840->status_reg |= 0x80;
		else
			ptm6840->status_reg &= ~0x80;

		devcb_call_write_line(&ptm6840->irq_func, ptm6840->IRQ);
	}
}

/*-------------------------------------------------
    compute_counter - Compute Counter
-------------------------------------------------*/

static UINT16 compute_counter( const device_config *device, int counter )
{
	ptm6840_state *ptm6840 = get_safe_token(device);
	int clock;
	int remaining = 0;

	/* If there's no timer, return the count */
	if (!ptm6840->enabled[counter])
	{
		PLOG(("MC6840 #%s: read counter(%d): %d\n", device->tag, counter, ptm6840->counter[counter]));
		return ptm6840->counter[counter];
	}

	/* determine the clock frequency for this timer */
	if (ptm6840->control_reg[counter] & 0x02)
	{
		clock = ptm6840->internal_clock;
		PLOG(("MC6840 #%s: %d internal clock freq %d \n", device->tag, counter, clock));
	}
	else
	{
		clock = ptm6840->external_clock[counter];
		PLOG(("MC6840 #%s: %d external clock freq %d \n", device->tag, counter, clock));
	}
	/* See how many are left */
	remaining = attotime_to_double(attotime_mul(timer_timeleft(ptm6840->timer[counter]), clock));

	/* Adjust the count for dual byte mode */
	if (ptm6840->control_reg[counter] & 0x04)
	{
		int divisor = (ptm6840->counter[counter] & 0xff) + 1;
		int msb = remaining / divisor;
		int lsb = remaining % divisor;
		remaining = (msb << 8) | lsb;
	}
	PLOG(("MC6840 #%s: read counter(%d): %d\n", device->tag, counter, remaining));
	return remaining;
}

/*-------------------------------------------------
    reload_count - Reload Counter
-------------------------------------------------*/

static void reload_count( const device_config *device, int idx )
{
	ptm6840_state *ptm6840 = get_safe_token(device);
	int clock;
	int count;
	attotime duration;

	/* Copy the latched value in */
	ptm6840->counter[idx] = ptm6840->latch[idx];

	/* Determine the clock frequency for this timer */
	if (ptm6840->control_reg[idx] & 0x02)
	{
		clock = ptm6840->internal_clock;
		PLOG(("MC6840 #%s: %d internal clock freq %d \n", device->tag, idx, clock));
	}
	else
	{
		clock = ptm6840->external_clock[idx];
		PLOG(("MC6840 #%s: %d external clock freq %d \n", device->tag, idx, clock));
	}

	/* Determine the number of clock periods before we expire */
	count = ptm6840->counter[idx];
	if (ptm6840->control_reg[idx] & 0x04)
		count = ((count >> 8) + 1) * ((count & 0xff) + 1);
	else
		count = count + 1;

	ptm6840->fired[idx] = 0;

	if ((ptm6840->mode[idx] == 4) || (ptm6840->mode[idx] == 6))
	{
		ptm6840->output[idx] = 1;
		if (ptm6840->out_func[idx].write != NULL)
			devcb_call_write8(&ptm6840->out_func[idx], 0, ptm6840->output[idx]);
	}

	/* Set the timer */
	PLOG(("MC6840 #%s: reload_count(%d): clock = %d  count = %d\n", device->tag, idx, clock, count));

	duration = attotime_mul(ATTOTIME_IN_HZ(clock), count);
	if (idx == 2)
		duration = attotime_mul(duration, ptm6840->t3_divisor);

	PLOG(("MC6840 #%s: reload_count(%d): output = %lf\n", device->tag, idx, attotime_to_double(duration)));

	if (!(ptm6840->control_reg[idx] & 0x02))
	{
		if (!ptm6840->external_clock[idx])
		{
			ptm6840->enabled[idx] = 0;
			timer_enable(ptm6840->timer[idx],FALSE);
		}
	}
	else
	{
		ptm6840->enabled[idx] = 1;
		timer_adjust_oneshot(ptm6840->timer[idx], duration, 0);
		timer_enable(ptm6840->timer[idx], TRUE);
	}
}


/*-------------------------------------------------
    ptm6840_read - Read Timer
-------------------------------------------------*/

READ8_DEVICE_HANDLER( ptm6840_read )
{
	ptm6840_state *ptm6840 = get_safe_token(device);
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
			PLOG(("%s: MC6840 #%s: Status read = %04X\n", cpuexec_describe_context(device->machine), device->tag, ptm6840->status_reg));
			ptm6840->status_read_since_int |= ptm6840->status_reg & 0x07;
			val = ptm6840->status_reg;
			break;
		}

		case PTM_6840_MSBBUF1:
		case PTM_6840_MSBBUF2:
		case PTM_6840_MSBBUF3:
		{
			int idx = (offset - 2) / 2;
			int result = compute_counter(device, idx);

			/* Clear the interrupt if the status has been read */
			if (ptm6840->status_read_since_int & (1 << idx))
			{
				ptm6840->status_reg &= ~(1 << idx);
				update_interrupts(device);
			}

			ptm6840->lsb_buffer = result & 0xff;

			PLOG(("%s: MC6840 #%s: Counter %d read = %04X\n", cpuexec_describe_context(device->machine), device->tag, idx, result >> 8));
			val = result >> 8;
			break;
		}

		case PTM_6840_LSB1:
		case PTM_6840_LSB2:
		case PTM_6840_LSB3:
		{
			val = ptm6840->lsb_buffer;
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

WRITE8_DEVICE_HANDLER( ptm6840_write )
{
	ptm6840_state *ptm6840 = get_safe_token(device);
	int idx;
	int i;
	UINT8 diffs;

	switch ( offset )
	{
		case PTM_6840_CTRL1:
		case PTM_6840_CTRL2:
		{
			idx = (offset == 1) ? 1 : (ptm6840->control_reg[1] & 0x01) ? 0 : 2;
			diffs = data ^ ptm6840->control_reg[idx];
			ptm6840->t3_divisor = (ptm6840->control_reg[2] & 0x01) ? 8 : 1;
			ptm6840->mode[idx] = (data >> 3) & 0x07;
			ptm6840->control_reg[idx] = data;

			PLOG(("MC6840 #%s : Control register %d selected\n", device->tag, idx));
			PLOG(("operation mode   = %s\n", opmode[ ptm6840->mode[idx] ]));
			PLOG(("value            = %04X\n", ptm6840->control_reg[idx]));
			PLOG(("t3divisor        = %d\n", ptm6840->t3_divisor));

			if (!(ptm6840->control_reg[idx] & 0x80 ))
			{
				/* Output cleared */
				if (ptm6840->out_func[idx].write != NULL)
					devcb_call_write8(&ptm6840->out_func[idx], 0, 0);
			}
			/* Reset? */
			if (idx == 0 && (diffs & 0x01))
			{
				/* Holding reset down */
				if (data & 0x01)
				{
					PLOG(("MC6840 #%s : Timer reset\n", device->tag));
					for (i = 0; i < 3; i++)
					{
						timer_enable(ptm6840->timer[i], FALSE);
						ptm6840->enabled[i] = 0;
					}
				}
				/* Releasing reset */
				else
				{
					for (i = 0; i < 3; i++)
						reload_count(device, i);
				}

				ptm6840->status_reg = 0;
				update_interrupts(device);

				/* Changing the clock source? (e.g. Zwackery) */
				if (diffs & 0x02)
					reload_count(device, idx);
			}
			break;
		}

		case PTM_6840_MSBBUF1:
		case PTM_6840_MSBBUF2:
		case PTM_6840_MSBBUF3:
		{
			PLOG(("MC6840 #%s msbbuf%d = %02X\n", device->tag, offset / 2, data));
			ptm6840->msb_buffer = data;
			break;
		}

		case PTM_6840_LSB1:
		case PTM_6840_LSB2:
		case PTM_6840_LSB3:
		{
			idx = (offset - 3) / 2;
			ptm6840->latch[idx] = (ptm6840->msb_buffer << 8) | (data & 0xff);

			/* Clear the interrupt */
			ptm6840->status_reg &= ~(1 << idx);
			update_interrupts(device);

			/* Reload the count if in an appropriate mode */
			if (!(ptm6840->control_reg[idx] & 0x10))
				reload_count(device,idx);

			PLOG(("%s:MC6840 #%s: Counter %d latch = %04X\n", cpuexec_describe_context(device->machine), device->tag, idx, ptm6840->latch[idx]));
			break;
		}
	}
}

/*-------------------------------------------------
    ptm6840_timeout - Called if timer is mature
-------------------------------------------------*/

static void ptm6840_timeout( const device_config *device, int idx )
{
	ptm6840_state *ptm6840 = get_safe_token(device);

	PLOG(("**ptm6840 %s t%d timeout**\n", device->tag, idx + 1));

	if ( ptm6840->control_reg[idx] & 0x40 )
	{
		/* Interrupt enabled */
		ptm6840->status_reg |= (1 << idx);
		ptm6840->status_read_since_int &= ~(1 << idx);
		update_interrupts(device);
	}

	if ( ptm6840->control_reg[idx] & 0x80 )
	{
		if ((ptm6840->mode[idx] == 0)||(ptm6840->mode[idx] == 2))
		{
			ptm6840->output[idx] = ptm6840->output[idx] ? 0 : 1;
			PLOG(("**ptm6840 %s t%d output %d **\n", device->tag, idx + 1, ptm6840->output[idx]));

			if (ptm6840->out_func[idx].write != NULL)
				devcb_call_write8(&ptm6840->out_func[idx], 0, ptm6840->output[idx]);
		}
		if ((ptm6840->mode[idx] == 4)||(ptm6840->mode[idx] == 6))
		{
			if (!ptm6840->fired[idx])
			{
				ptm6840->output[idx] = 1;
				PLOG(("**ptm6840 %s t%d output %d **\n", device->tag, idx + 1, ptm6840->output[idx]));

				if (ptm6840->out_func[idx].write != NULL)
					devcb_call_write8(&ptm6840->out_func[idx], 0, ptm6840->output[idx]);

				/* No changes in output until reinit */
				ptm6840->fired[idx] = 1;
			}
		}
	}
	ptm6840->enabled[idx]= 0;
	reload_count(device, idx);
}

/*-------------------------------------------------
    TIMER_CALLBACKs for Timer 1, 2 & 3
-------------------------------------------------*/

static TIMER_CALLBACK( ptm6840_timer1_cb )
{
	const device_config *device = (const device_config *)ptr;
	ptm6840_timeout(device, 0);
}

static TIMER_CALLBACK( ptm6840_timer2_cb )
{
	const device_config *device = (const device_config *)ptr;
	ptm6840_timeout(device, 1);
}

static TIMER_CALLBACK( ptm6840_timer3_cb )
{
	const device_config *device = (const device_config *)ptr;
	ptm6840_timeout(device, 2);
}


/*-------------------------------------------------
    ptm6840_set_gate - set gate status (0 or 1)
-------------------------------------------------*/

INLINE void ptm6840_set_gate( const device_config *device, int state, int idx )
{
	ptm6840_state *ptm6840 = get_safe_token(device);

	if ( (ptm6840->mode[idx] == 0) || (ptm6840->mode[idx] == 2) || (ptm6840->mode[0] == 4) || (ptm6840->mode[idx] == 6) )
	{
		if (state == 0 && ptm6840->gate[idx])
			reload_count(device,idx);
	}
	ptm6840->gate[idx] = state;
}

/*-------------------------------------------------
    WRITE8_DEVICE_HANDLERs for Gate 1, 2 & 3
-------------------------------------------------*/

WRITE8_DEVICE_HANDLER( ptm6840_set_g1 )
{
	ptm6840_set_gate(device, data, 0);
}

WRITE8_DEVICE_HANDLER( ptm6840_set_g2 )
{
	ptm6840_set_gate(device, data, 1);
}

WRITE8_DEVICE_HANDLER( ptm6840_set_g3 )
{
	ptm6840_set_gate(device, data, 2);
}

/*-------------------------------------------------
    ptm6840_set_clock - set clock status (0 or 1)
-------------------------------------------------*/

INLINE void ptm6840_set_clock( const device_config *device, int state, int idx )
{
	ptm6840_state *ptm6840 = get_safe_token(device);

	ptm6840->clock[idx] = state;

	if (!(ptm6840->control_reg[idx] & 0x02))
	{
		if (state)
			ptm_tick(device, idx, 1);
	}
}

/*-------------------------------------------------
    WRITE8_DEVICE_HANDLERs for Clock 1, 2 & 3
-------------------------------------------------*/

WRITE8_DEVICE_HANDLER( ptm6840_set_c1 )
{
	ptm6840_set_clock(device, data, 0);
}

WRITE8_DEVICE_HANDLER( ptm6840_set_c2 )
{
	ptm6840_set_clock(device, data, 1);
}

WRITE8_DEVICE_HANDLER( ptm6840_set_c3 )
{
	ptm6840_set_clock(device, data, 2);
}


/*-------------------------------------------------
    ptm6840_get_count - get count value
-------------------------------------------------*/

UINT16 ptm6840_get_count( const device_config *device, int counter )
{
	return compute_counter(device, counter);
}

/*------------------------------------------------------------
    ptm6840_set_ext_clock - set external clock frequency
------------------------------------------------------------*/

void ptm6840_set_ext_clock( const device_config *device, int counter, int clock )
{
	ptm6840_state *ptm6840 = get_safe_token(device);

	ptm6840->external_clock[counter] = clock;

	if (!(ptm6840->control_reg[counter] & 0x02))
	{
		if (!ptm6840->external_clock[counter])
		{
			ptm6840->enabled[counter] = 0;
			timer_enable(ptm6840->timer[counter], FALSE);
		}
	}
	else
	{
		int count;
		attotime duration;

		/* Determine the number of clock periods before we expire */
		count = ptm6840->counter[counter];

		if (ptm6840->control_reg[counter] & 0x04)
			count = ((count >> 8) + 1) * ((count & 0xff) + 1);
		else
			count = count + 1;

		duration = attotime_mul(ATTOTIME_IN_HZ(clock), count);

		if (counter == 2)
			duration = attotime_mul(duration, ptm6840->t3_divisor);

		ptm6840->enabled[counter] = 1;
		timer_adjust_oneshot(ptm6840->timer[counter], duration, 0);
		timer_enable(ptm6840->timer[counter], TRUE);
	}
}

/*------------------------------------------------------------
    ptm6840_get_ext_clock - get external clock frequency
------------------------------------------------------------*/

int ptm6840_get_ext_clock( const device_config *device, int counter )
{
	ptm6840_state *ptm6840 = get_safe_token(device);
	return ptm6840->external_clock[counter];
}


/*-------------------------------------------------
    DEVICE_START( ptm6840 )
-------------------------------------------------*/

static DEVICE_START( ptm6840 )
{
	ptm6840_state *ptm6840 = get_safe_token(device);
	const ptm6840_interface *intf = get_interface(device);
	int i;

	ptm6840->internal_clock = intf->internal_clock;

	/* resolve callbacks */
	for (i = 0; i < 3; i++)
	{
		devcb_resolve_write8(&ptm6840->out_func[i], &intf->out_func[i], device);
	}

	for (i = 0; i < 3; i++)
	{
		if ( intf->external_clock[i] )
			ptm6840->external_clock[i] = intf->external_clock[i];
		else
			ptm6840->external_clock[i] = 1;
	}


	ptm6840->timer[0] = timer_alloc(device->machine, ptm6840_timer1_cb, (void *)device);
	ptm6840->timer[1] = timer_alloc(device->machine, ptm6840_timer2_cb, (void *)device);
	ptm6840->timer[2] = timer_alloc(device->machine, ptm6840_timer3_cb, (void *)device);

	for (i = 0; i < 3; i++)
	{
		timer_enable(ptm6840->timer[i], FALSE);
	}

	devcb_resolve_write_line(&ptm6840->irq_func, &intf->irq_func, device);

	/* register for state saving */
	state_save_register_device_item(device, 0, ptm6840->lsb_buffer);
	state_save_register_device_item(device, 0, ptm6840->msb_buffer);
	state_save_register_device_item(device, 0, ptm6840->status_read_since_int);
	state_save_register_device_item(device, 0, ptm6840->status_reg);
	state_save_register_device_item(device, 0, ptm6840->t3_divisor);
	state_save_register_device_item(device, 0, ptm6840->t3_scaler);
	state_save_register_device_item(device, 0, ptm6840->internal_clock);
	state_save_register_device_item(device, 0, ptm6840->IRQ);

	state_save_register_device_item_array(device, 0, ptm6840->control_reg);
	state_save_register_device_item_array(device, 0, ptm6840->output);
	state_save_register_device_item_array(device, 0, ptm6840->gate);
	state_save_register_device_item_array(device, 0, ptm6840->clock);
	state_save_register_device_item_array(device, 0, ptm6840->mode);
	state_save_register_device_item_array(device, 0, ptm6840->fired);
	state_save_register_device_item_array(device, 0, ptm6840->enabled);
	state_save_register_device_item_array(device, 0, ptm6840->external_clock);
	state_save_register_device_item_array(device, 0, ptm6840->counter);
	state_save_register_device_item_array(device, 0, ptm6840->latch);
}

/*-------------------------------------------------
    DEVICE_RESET( ptm6840 )
-------------------------------------------------*/

static DEVICE_RESET( ptm6840 )
{
	ptm6840_state *ptm6840 = get_safe_token(device);
	int i;

	ptm6840->control_reg[2]		 = 0;
	ptm6840->control_reg[1]		 = 0;
	ptm6840->control_reg[0]		 = 1;
	ptm6840->status_reg			 = 0;
	ptm6840->t3_divisor			 = 1;
	ptm6840->status_read_since_int = 0;
	ptm6840->IRQ                   = 0;

	for (i = 0; i < 3; i++)
	{
		ptm6840->counter[i] = 0xffff;
		ptm6840->latch[i]   = 0xffff;
		ptm6840->output[i]  = 0;
		ptm6840->fired[i]   = 0;
	}
}

/*-------------------------------------------------
    device definition
-------------------------------------------------*/

static const char *DEVTEMPLATE_SOURCE = __FILE__;

#define DEVTEMPLATE_ID(p,s)		p##ptm6840##s
#define DEVTEMPLATE_FEATURES	DT_HAS_START | DT_HAS_RESET
#define DEVTEMPLATE_NAME		"6840 PTM"
#define DEVTEMPLATE_FAMILY		"Motorola Programmable Timer Modules"
#define DEVTEMPLATE_CLASS		DEVICE_CLASS_PERIPHERAL
#include "devtempl.h"
