/**********************************************************************


    Motorola 6840 PTM interface and emulation

    This function is a simple emulation of up to 4 MC6840
    Programmable Timer Modules

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

**********************************************************************/

#include "driver.h"
#include "6840ptm.h"

// Defines /////////////////////////////////////////////////////////////

#define PTMVERBOSE 0
#define PLOG(x) do { if (PTMVERBOSE) logerror x; } while (0)

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


typedef struct _ptm6840
{
	const ptm6840_interface *intf;

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

	int internal_clock;
	int external_clock[3];

	/* Each PTM has 3 timers */
	emu_timer *timer[3];

	UINT16 latch[3];
	UINT16 counter[3];
} ptm6840;

// Local prototypes ///////////////////////////////////////////////////////

static void ptm6840_timeout(running_machine *machine, int which, int idx);
static TIMER_CALLBACK( ptm6840_timer_cb );

// Local vars /////////////////////////////////////////////////////////////

static ptm6840 ptm[PTM_6840_MAX];

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

///////////////////////////////////////////////////////////////////////////
//                                                                       //
// Get enabled status                                                    //
//                                                                       //
///////////////////////////////////////////////////////////////////////////

int ptm6840_get_status(int which, int clock)
{
	ptm6840 *p = ptm + which;
	return p->enabled[clock - 1];
}

///////////////////////////////////////////////////////////////////////////
//                                                                       //
// ptm6840_get_irq: get IRQ state                                        //
//                                                                       //
///////////////////////////////////////////////////////////////////////////

int ptm6840_get_irq(int which)
{
	ptm6840 *p = ptm + which;
	return p->IRQ;
}

///////////////////////////////////////////////////////////////////////////
//                                                                       //
// Subtract from Counter                                                 //
//                                                                       //
///////////////////////////////////////////////////////////////////////////

static void subtract_from_counter(running_machine *machine, int which, int counter, int count)
{
	int clock;
	attotime duration;

	ptm6840 *currptr = ptm + which;

	/* Determine the clock frequency for this timer */
	if (currptr->control_reg[counter] & 0x02)
		clock = currptr->internal_clock;
	else
		clock = currptr->external_clock[counter];

	/* Dual-byte mode */
	if (currptr->control_reg[counter] & 0x04)
	{
		int lsb = currptr->counter[counter] & 0xff;
		int msb = currptr->counter[counter] >> 8;

		/* Count the clocks */
		lsb -= count;

		/* Loop while we're less than zero */
		while (lsb < 0)
		{
			/* Borrow from the MSB */
			lsb += (currptr->latch[counter] & 0xff) + 1;
			msb--;

			/* If MSB goes less than zero, we've expired */
			if (msb < 0)
			{
				ptm6840_timeout(machine, which, counter);
				msb = (currptr->latch[counter] >> 8) + 1;
			}
		}

		/* Store the result */
		currptr->counter[counter] = (msb << 8) | lsb;
	}

	/* Word mode */
	else
	{
		int word = currptr->counter[counter];

		/* Count the clocks */
		word -= count;

		/* loop while we're less than zero */
		while (word < 0)
		{
			/* Borrow from the MSB */
			word += currptr->latch[counter] + 1;

			/* We've expired */
			ptm6840_timeout(machine, which, counter);
		}

		/* Store the result */
		currptr->counter[counter] = word;
	}

	if (currptr->enabled[counter])
	{
		duration = attotime_mul(ATTOTIME_IN_HZ(clock), currptr->counter[counter]);

		if (counter == 2)
			duration = attotime_mul(duration, currptr->t3_divisor);

		timer_adjust_oneshot(currptr->timer[counter], duration, which);
	}
}

static void ptm_tick(running_machine *machine, int which, int counter, int count)
{
	ptm6840 *currptr = ptm + which;

	if (counter == 2)
	{
		currptr->t3_scaler += count;

		if ( currptr->t3_scaler > currptr->t3_divisor - 1)
		{
			subtract_from_counter(machine, which, counter, 1);
			currptr->t3_scaler = 0;
		}
	}
	else
	{
		subtract_from_counter(machine, which, counter, count);
	}
}

///////////////////////////////////////////////////////////////////////////
//                                                                       //
// Update Internal Interrupts                                            //
//                                                                       //
///////////////////////////////////////////////////////////////////////////

INLINE void update_interrupts(running_machine *machine, int which)
{
	int new_state;
	ptm6840 *currptr = ptm + which;

	new_state = ((currptr->status_reg & 0x01) && (currptr->control_reg[0] & 0x40)) ||
				((currptr->status_reg & 0x02) && (currptr->control_reg[1] & 0x40)) ||
				((currptr->status_reg & 0x04) && (currptr->control_reg[2] & 0x40));

//  if (new_state != currptr->IRQ)
	{
		currptr->IRQ = new_state;

		if (currptr->IRQ)
			currptr->status_reg |= 0x80;
		else
			currptr->status_reg &= ~0x80;

		if (currptr->intf->irq_func)
			(currptr->intf->irq_func)(machine, currptr->IRQ);
	}
}

///////////////////////////////////////////////////////////////////////////
//                                                                       //
// Compute Counter                                                       //
//                                                                       //
///////////////////////////////////////////////////////////////////////////

static UINT16 compute_counter(int which, int counter)
{
	ptm6840 *currptr = ptm + which;

	int clock;
	int remaining=0;

	/* If there's no timer, return the count */
	if (!currptr->enabled[counter])
	{
		PLOG(("MC6840 #%d: read counter(%d): %d\n", which, counter, currptr->counter[counter]));
		return currptr->counter[counter];
	}

	/* determine the clock frequency for this timer */
	if (currptr->control_reg[counter] & 0x02)
	{
		clock = currptr->internal_clock;
		PLOG(("MC6840 #%d: %d internal clock freq %d \n", which, counter, clock));
	}
	else
	{
		clock = currptr->external_clock[counter];
		PLOG(("MC6840 #%d: %d external clock freq %d \n", which, counter, clock));
	}
	/* See how many are left */
	remaining = attotime_to_double(attotime_mul(timer_timeleft(currptr->timer[counter]), clock));

	/* Adjust the count for dual byte mode */
	if (currptr->control_reg[counter] & 0x04)
	{
		int divisor = (currptr->counter[counter] & 0xff) + 1;
		int msb = remaining / divisor;
		int lsb = remaining % divisor;
		remaining = (msb << 8) | lsb;
	}
	PLOG(("MC6840 #%d: read counter(%d): %d\n", which, counter, remaining));
	return remaining;
}

///////////////////////////////////////////////////////////////////////////
//                                                                       //
// Reload Counter                                                        //
//                                                                       //
///////////////////////////////////////////////////////////////////////////

static void reload_count(running_machine *machine, int which, int idx)
{
	/* temporary hack until this is converted to a device */
	const address_space *space = cpu_get_address_space(machine->cpu[0], ADDRESS_SPACE_PROGRAM);
	int clock;
	int count;
	attotime duration;
	ptm6840 *currptr = ptm + which;

	/* Copy the latched value in */
	currptr->counter[idx] = currptr->latch[idx];

	/* Determine the clock frequency for this timer */
	if (currptr->control_reg[idx] & 0x02)
	{
		clock = currptr->internal_clock;
		PLOG(("MC6840 #%d: %d internal clock freq %d \n", which,idx, clock));
	}
	else
	{
		clock = currptr->external_clock[idx];
		PLOG(("MC6840 #%d: %d external clock freq %d \n", which,idx, clock));
	}

	/* Determine the number of clock periods before we expire */
	count = currptr->counter[idx];
	if (currptr->control_reg[idx] & 0x04)
		count = ((count >> 8) + 1) * ((count & 0xff) + 1);
	else
		count = count + 1;

	currptr->fired[idx]=0;

	if ((currptr->mode[idx] == 4)||(currptr->mode[idx] == 6))
	{
		currptr->output[idx] = 1;
		if ( currptr->intf->out_func[idx] ) currptr->intf->out_func[idx](space, 0, currptr->output[idx]);
	}

	/* Set the timer */
	PLOG(("MC6840 #%d: reload_count(%d): clock = %d  count = %d\n", which, idx, clock, count));

	duration = attotime_mul(ATTOTIME_IN_HZ(clock), count);
	if (idx == 2) duration = attotime_mul(duration, currptr->t3_divisor);
	PLOG(("MC6840 #%d: reload_count(%d): output = %lf\n", which, idx, attotime_to_double(duration)));

	if (!currptr->control_reg[idx] & 0x02)
	{
		if (!currptr->intf->external_clock[idx])
		{
			currptr->enabled[idx] = 0;
			timer_enable(currptr->timer[idx],FALSE);
		}
	}
	else
	{
		currptr->enabled[idx] = 1;
		timer_adjust_oneshot(currptr->timer[idx], duration, which);
		timer_enable(currptr->timer[idx],TRUE);
	}
}

///////////////////////////////////////////////////////////////////////////
//                                                                       //
// Configure Timer                                                       //
//                                                                       //
///////////////////////////////////////////////////////////////////////////

void ptm6840_config(running_machine *machine, int which, const ptm6840_interface *intf)
{
	int i;
	ptm6840 *currptr = ptm + which;

	assert_always(mame_get_phase(machine) == MAME_PHASE_INIT, "Can only call ptm6840_config at init time!");
	assert_always((which >= 0) && (which < PTM_6840_MAX), "ptm6840_config called on an invalid PTM!");
	assert_always(intf, "ptm6840_config called with an invalid interface!");
	ptm[which].intf = intf;
	ptm[which].internal_clock = currptr->intf->internal_clock;

	for (i = 0; i < 3; i++)
	{
		if ( currptr->intf->external_clock[i] )
			ptm[which].external_clock[i] = currptr->intf->external_clock[i];
		else
			ptm[which].external_clock[i] = 1;
	}

	for (i = 0; i < 3; i++)
	{
		ptm[which].timer[i] = timer_alloc(machine, ptm6840_timer_cb, (void*)(FPTR)i);
		timer_enable(ptm[which].timer[i], FALSE);
	}

	state_save_register_item(machine, "6840ptm", NULL, which, currptr->lsb_buffer);
	state_save_register_item(machine, "6840ptm", NULL, which, currptr->msb_buffer);
	state_save_register_item(machine, "6840ptm", NULL, which, currptr->status_read_since_int);
	state_save_register_item(machine, "6840ptm", NULL, which, currptr->status_reg);
	state_save_register_item(machine, "6840ptm", NULL, which, currptr->t3_divisor);
	state_save_register_item(machine, "6840ptm", NULL, which, currptr->t3_scaler);
	state_save_register_item(machine, "6840ptm", NULL, which, currptr->internal_clock);
	state_save_register_item(machine, "6840ptm", NULL, which, currptr->IRQ);

	state_save_register_item_array(machine, "6840ptm", NULL, which, currptr->control_reg);
	state_save_register_item_array(machine, "6840ptm", NULL, which, currptr->output);
	state_save_register_item_array(machine, "6840ptm", NULL, which, currptr->gate);
	state_save_register_item_array(machine, "6840ptm", NULL, which, currptr->clock);
	state_save_register_item_array(machine, "6840ptm", NULL, which, currptr->mode);
	state_save_register_item_array(machine, "6840ptm", NULL, which, currptr->fired);
	state_save_register_item_array(machine, "6840ptm", NULL, which, currptr->enabled);
	state_save_register_item_array(machine, "6840ptm", NULL, which, currptr->external_clock);
	state_save_register_item_array(machine, "6840ptm", NULL, which, currptr->counter);
	state_save_register_item_array(machine, "6840ptm", NULL, which, currptr->latch);

	ptm6840_reset(which);
}

///////////////////////////////////////////////////////////////////////////
//                                                                       //
// Reset Timer                                                           //
//                                                                       //
///////////////////////////////////////////////////////////////////////////

void ptm6840_reset(int which)
{
	int i;

	ptm[which].control_reg[2]		 = 0;
	ptm[which].control_reg[1]		 = 0;
	ptm[which].control_reg[0]		 = 1;
	ptm[which].status_reg			 = 0;
	ptm[which].t3_divisor			 = 1;
	ptm[which].status_read_since_int = 0;
	ptm[which].IRQ                   = 0;

	for (i = 0; i < 3; i++)
	{
		ptm[which].counter[i] = 0xffff;
		ptm[which].latch[i]   = 0xffff;
		ptm[which].output[i]  = 0;
		ptm[which].fired[i]   = 0;
	}
}

///////////////////////////////////////////////////////////////////////////
//                                                                       //
// Read Timer                                                            //
//                                                                       //
///////////////////////////////////////////////////////////////////////////

int ptm6840_read(running_machine *machine, int which, int offset)
{
	int val;
	ptm6840 *currptr = ptm + which;

	switch ( offset )
	{
		case PTM_6840_CTRL1:
		{
			val = 0;
			break;
		}

		case PTM_6840_STATUS:
		{
			PLOG(("%06X: MC6840 #%d: Status read = %04X\n", cpu_get_previouspc(machine->activecpu), which, currptr->status_reg));
			currptr->status_read_since_int |= currptr->status_reg & 0x07;
			val = currptr->status_reg;
			break;
		}

		case PTM_6840_MSBBUF1:
		case PTM_6840_MSBBUF2:
		case PTM_6840_MSBBUF3:
		{
			int idx = (offset - 2) / 2;
			int result = compute_counter(which, idx);

			/* Clear the interrupt if the status has been read */
			if (currptr->status_read_since_int & (1 << idx))
			{
				currptr->status_reg &= ~(1 << idx);
				update_interrupts(machine, which);
			}

			currptr->lsb_buffer = result & 0xff;

			PLOG(("%06X: MC6840 #%d: Counter %d read = %04X\n", cpu_get_previouspc(machine->activecpu), which, idx, result >> 8));
			val = result >> 8;
			break;
		}

		case PTM_6840_LSB1:
		case PTM_6840_LSB2:
		case PTM_6840_LSB3:
		{
			val = currptr->lsb_buffer;
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

///////////////////////////////////////////////////////////////////////////
//                                                                       //
// Write Timer                                                           //
//                                                                       //
///////////////////////////////////////////////////////////////////////////

void ptm6840_write (running_machine *machine, int which, int offset, int data)
{
	/* temporary hack until this is converted to a device */
	const address_space *space = cpu_get_address_space(machine->cpu[0], ADDRESS_SPACE_PROGRAM);
	ptm6840 *currptr = ptm + which;

	int idx;
	int i;
	UINT8 diffs;

	switch ( offset )
	{
		case PTM_6840_CTRL1:
		case PTM_6840_CTRL2:
		{
			idx = (offset == 1) ? 1 : (currptr->control_reg[1] & 0x01) ? 0 : 2;
			diffs = data ^ currptr->control_reg[idx];
			currptr->t3_divisor = (currptr->control_reg[2] & 0x01) ? 8 : 1;
			currptr->mode[idx] = (data >> 3) & 0x07;
			currptr->control_reg[idx] = data;

			PLOG(("MC6840 #%d : Control register %d selected\n", which, idx));
			PLOG(("operation mode   = %s\n", opmode[ currptr->mode[idx] ]));
			PLOG(("value            = %04X\n", currptr->control_reg[idx]));
			PLOG(("t3divisor        = %d\n", currptr->t3_divisor));

			if (!(currptr->control_reg[idx] & 0x80 ))
			{
				/* Output cleared */
				if ( currptr->intf )
				{
					if ( currptr->intf->out_func[idx] )
						currptr->intf->out_func[idx](space, 0, 0);
				}
			}
			/* Reset? */
			if (idx == 0 && (diffs & 0x01))
			{
				/* Holding reset down */
				if (data & 0x01)
				{
					PLOG(("MC6840 #%d : Timer reset\n", which));
					for (i = 0; i < 3; i++)
					{
						timer_enable(currptr->timer[i], FALSE);
						currptr->enabled[i] = 0;
					}
				}
				/* Releasing reset */
				else
				{
					for (i = 0; i < 3; i++)
						reload_count(machine, which, i);
				}

				currptr->status_reg = 0;
				update_interrupts(machine, which);

				/* Changing the clock source? (e.g. Zwackery) */
				if (diffs & 0x02)
					reload_count(machine, which, idx);
			}
			break;
		}

		case PTM_6840_MSBBUF1:
		case PTM_6840_MSBBUF2:
		case PTM_6840_MSBBUF3:
		{
			PLOG(("MC6840 #%d msbbuf%d = %02X\n", which, offset / 2, data));
			currptr->msb_buffer = data;
			break;
		}

		case PTM_6840_LSB1:
		case PTM_6840_LSB2:
		case PTM_6840_LSB3:
		{
			idx = (offset - 3) / 2;
			currptr->latch[idx] = (currptr->msb_buffer << 8) | (data & 0xff);

			/* Clear the interrupt */
			currptr->status_reg &= ~(1 << idx);
			update_interrupts(machine, which);

			/* Reload the count if in an appropriate mode */
			if (!(currptr->control_reg[idx] & 0x10))
				reload_count(machine, which,idx);

			PLOG(("%06X:MC6840 #%d: Counter %d latch = %04X\n", cpu_get_previouspc(machine->activecpu), which, idx, currptr->latch[idx]));
			break;
		}
	}
}

///////////////////////////////////////////////////////////////////////////
//                                                                       //
// ptm6840_timeout: called if timer is mature                            //
//                                                                       //
///////////////////////////////////////////////////////////////////////////

static void ptm6840_timeout(running_machine *machine, int which, int idx)
{
	/* temporary hack until this is converted to a device */
	const address_space *space = cpu_get_address_space(machine->cpu[0], ADDRESS_SPACE_PROGRAM);
	ptm6840 *p = ptm + which;

	PLOG(("**ptm6840 %d t%d timeout**\n", which, idx + 1));

	if ( p->control_reg[idx] & 0x40 )
	{
		/* Interrupt enabled */
		p->status_reg |= (1 << idx);
		p->status_read_since_int &= ~(1 << idx);
		update_interrupts(machine, which);
	}

	if ( p->control_reg[idx] & 0x80 )
	{
		/* Output enabled */
		if ( p->intf )
		{
			if ((p->mode[idx] == 0)||(p->mode[idx] == 2))
			{
				p->output[idx] = p->output[idx]?0:1;
				PLOG(("**ptm6840 %d t%d output %d **\n", which, idx + 1, p->output[idx]));

				if ( p->intf->out_func[idx] )
					p->intf->out_func[idx](space, 0, p->output[idx]);
			}
			if ((p->mode[idx] == 4)||(p->mode[idx] == 6))
			{
				if (!p->fired[idx])
				{
					p->output[idx] = 1;
					PLOG(("**ptm6840 %d t%d output %d **\n", which, idx + 1, p->output[idx]));

					if ( p->intf->out_func[idx] )
						p->intf->out_func[idx](space, 0, p->output[idx]);

					/* No changes in output until reinit */
					p->fired[idx] = 1;
				}
			}
		}
	}
	p->enabled[idx]= 0;
	reload_count(machine, which, idx);
}

static TIMER_CALLBACK( ptm6840_timer_cb ) { ptm6840_timeout(machine, param, (int)(FPTR)ptr); }

///////////////////////////////////////////////////////////////////////////
//                                                                       //
// ptm6840_set_gate: set gate status (0 or 1)                            //
//                                                                       //
///////////////////////////////////////////////////////////////////////////

INLINE void ptm6840_set_gate(running_machine *machine, int which, int state, int idx)
{
	ptm6840 *p = ptm + which;

	if ( (p->mode[idx] == 0) || (p->mode[idx] == 2) || (p->mode[0] == 4) || (p->mode[idx] == 6) )
	{
		if (state == 0 && p->gate[idx])
			reload_count(machine, which,idx);
	}
	p->gate[idx] = state;
}

void ptm6840_set_g1(running_machine *machine, int which, int state) { ptm6840_set_gate(machine, which, state, 0); }
void ptm6840_set_g2(running_machine *machine, int which, int state) { ptm6840_set_gate(machine, which, state, 1); }
void ptm6840_set_g3(running_machine *machine, int which, int state) { ptm6840_set_gate(machine, which, state, 2); }

///////////////////////////////////////////////////////////////////////////
//                                                                       //
// ptm6840_set_clock: set clock status (0 or 1)                          //
//                                                                       //
///////////////////////////////////////////////////////////////////////////

INLINE void ptm6840_set_clock(running_machine *machine, int which, int state, int idx)
{
	ptm6840 *p = ptm + which;

	p->clock[idx] = state;

	if (!(p->control_reg[idx] & 0x02))
	{
		if (state)
			ptm_tick(machine, which, idx, 1);
	}
}

///////////////////////////////////////////////////////////////////////////
//                                                                       //
// ptm6840_get_count: get count value                                    //
//                                                                       //
///////////////////////////////////////////////////////////////////////////

UINT16 ptm6840_get_count(int which,int counter)
{
	return compute_counter(which, counter);
}

///////////////////////////////////////////////////////////////////////////
//                                                                       //
// ptm6840_set_ext_clock: set external clock frequency                   //
//                                                                       //
///////////////////////////////////////////////////////////////////////////

void ptm6840_set_ext_clock(int which, int counter, int clock)
{
	ptm6840 *currptr = ptm + which;
	currptr->external_clock[counter] = clock;

	if (!currptr->control_reg[counter] & 0x02)
	{
		if (!currptr->intf->external_clock[counter])
		{
			currptr->enabled[counter] = 0;
			timer_enable(currptr->timer[counter], FALSE);
		}
	}
	else
	{
		int count;
		attotime duration;

		/* Determine the number of clock periods before we expire */
		count = currptr->counter[counter];

		if (currptr->control_reg[counter] & 0x04)
			count = ((count >> 8) + 1) * ((count & 0xff) + 1);
		else
			count = count + 1;

		duration = attotime_mul(ATTOTIME_IN_HZ(clock), count);

		if (counter == 2)
			duration = attotime_mul(duration, currptr->t3_divisor);

		currptr->enabled[counter] = 1;
		timer_adjust_oneshot(currptr->timer[counter], duration, which);
		timer_enable(currptr->timer[counter], TRUE);
	}
}

///////////////////////////////////////////////////////////////////////////
//                                                                       //
// ptm6840_get_ext_clock: get external clock frequency                   //
//                                                                       //
///////////////////////////////////////////////////////////////////////////

int ptm6840_get_ext_clock(int which, int counter)
{
	ptm6840 *currptr = ptm + which;
	return currptr->external_clock[counter];
}

void ptm6840_set_c1(running_machine *machine, int which, int state) { ptm6840_set_clock(machine, which, state, 0); }
void ptm6840_set_c2(running_machine *machine, int which, int state) { ptm6840_set_clock(machine, which, state, 1); }
void ptm6840_set_c3(running_machine *machine, int which, int state) { ptm6840_set_clock(machine, which, state, 2); }

/////////////////////////////////////////////////////////////////////////////////

READ8_HANDLER( ptm6840_0_r ) { return ptm6840_read(space->machine, 0, offset); }
READ8_HANDLER( ptm6840_1_r ) { return ptm6840_read(space->machine, 1, offset); }
READ8_HANDLER( ptm6840_2_r ) { return ptm6840_read(space->machine, 2, offset); }
READ8_HANDLER( ptm6840_3_r ) { return ptm6840_read(space->machine, 3, offset); }

WRITE8_HANDLER( ptm6840_0_w ) { ptm6840_write(space->machine, 0, offset, data); }
WRITE8_HANDLER( ptm6840_1_w ) { ptm6840_write(space->machine, 1, offset, data); }
WRITE8_HANDLER( ptm6840_2_w ) { ptm6840_write(space->machine, 2, offset, data); }
WRITE8_HANDLER( ptm6840_3_w ) { ptm6840_write(space->machine, 3, offset, data); }

READ16_HANDLER( ptm6840_0_msb_r ) { return ptm6840_read(space->machine, 0, offset) << 8; }
READ16_HANDLER( ptm6840_1_msb_r ) { return ptm6840_read(space->machine, 1, offset) << 8; }
READ16_HANDLER( ptm6840_2_msb_r ) { return ptm6840_read(space->machine, 2, offset) << 8; }
READ16_HANDLER( ptm6840_3_msb_r ) { return ptm6840_read(space->machine, 3, offset) << 8; }

WRITE16_HANDLER( ptm6840_0_msb_w ) { if (ACCESSING_BITS_8_15) ptm6840_write(space->machine, 0, offset, data >> 8); }
WRITE16_HANDLER( ptm6840_1_msb_w ) { if (ACCESSING_BITS_8_15) ptm6840_write(space->machine, 1, offset, data >> 8); }
WRITE16_HANDLER( ptm6840_2_msb_w ) { if (ACCESSING_BITS_8_15) ptm6840_write(space->machine, 2, offset, data >> 8); }
WRITE16_HANDLER( ptm6840_3_msb_w ) { if (ACCESSING_BITS_8_15) ptm6840_write(space->machine, 3, offset, data >> 8); }

READ16_HANDLER( ptm6840_0_lsb_r ) { return ptm6840_read(space->machine, 0, offset); }
READ16_HANDLER( ptm6840_1_lsb_r ) { return ptm6840_read(space->machine, 1, offset); }
READ16_HANDLER( ptm6840_2_lsb_r ) { return ptm6840_read(space->machine, 2, offset); }
READ16_HANDLER( ptm6840_3_lsb_r ) { return ptm6840_read(space->machine, 3, offset); }

WRITE16_HANDLER( ptm6840_0_lsb_w ) { if (ACCESSING_BITS_0_7) ptm6840_write(space->machine, 0, offset, data & 0xff); }
WRITE16_HANDLER( ptm6840_1_lsb_w ) { if (ACCESSING_BITS_0_7) ptm6840_write(space->machine, 1, offset, data & 0xff); }
WRITE16_HANDLER( ptm6840_2_lsb_w ) { if (ACCESSING_BITS_0_7) ptm6840_write(space->machine, 2, offset, data & 0xff); }
WRITE16_HANDLER( ptm6840_3_lsb_w ) { if (ACCESSING_BITS_0_7) ptm6840_write(space->machine, 3, offset, data & 0xff); }
