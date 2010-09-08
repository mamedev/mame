/***************************************************************************

    Midway MCR system

***************************************************************************/

#include "emu.h"
#include "audio/mcr.h"
#include "includes/mcr68.h"

#define VERBOSE 0
#define LOG(x) do { if (VERBOSE) logerror x; } while (0)


/*************************************
 *
 *  Global variables
 *
 *************************************/

attotime mcr68_timing_factor;



/*************************************
 *
 *  Statics
 *
 *************************************/

static UINT8 m6840_status;
static UINT8 m6840_status_read_since_int;
static UINT8 m6840_msb_buffer;
static UINT8 m6840_lsb_buffer;
static struct counter_state
{
	UINT8			control;
	UINT16			latch;
	UINT16			count;
	emu_timer *	timer;
	UINT8			timer_active;
	attotime		period;
} m6840_state[3];

/* MCR/68k interrupt states */
static UINT8 m6840_irq_state;
static UINT8 m6840_irq_vector;
static UINT8 v493_irq_state;
static UINT8 v493_irq_vector;

static timer_fired_func v493_callback;

static UINT8 zwackery_sound_data;

static attotime m6840_counter_periods[3];
static attotime m6840_internal_counter_period;	/* 68000 CLK / 10 */


/*************************************
 *
 *  Function prototypes
 *
 *************************************/

static void subtract_from_counter(running_machine *machine, int counter, int count);

static TIMER_CALLBACK( mcr68_493_callback );
static TIMER_CALLBACK( zwackery_493_callback );

static WRITE8_DEVICE_HANDLER( zwackery_pia0_w );
static WRITE8_DEVICE_HANDLER( zwackery_pia1_w );
static WRITE_LINE_DEVICE_HANDLER( zwackery_ca2_w );
static WRITE_LINE_DEVICE_HANDLER( zwackery_pia_irq );

static void reload_count(int counter);
static TIMER_CALLBACK( counter_fired_callback );



/*************************************
 *
 *  6821 PIA declarations
 *
 *************************************/

static READ8_DEVICE_HANDLER( zwackery_port_1_r )
{
	UINT8 ret = input_port_read(device->machine, "IN1");

	pia6821_set_port_a_z_mask(device, ret);

	return ret;
}


static READ8_DEVICE_HANDLER( zwackery_port_3_r )
{
	UINT8 ret = input_port_read(device->machine, "IN3");

	pia6821_set_port_a_z_mask(device, ret);

	return ret;
}


const pia6821_interface zwackery_pia0_intf =
{
	DEVCB_NULL,		/* port A in */
	DEVCB_INPUT_PORT("IN0"),		/* port B in */
	DEVCB_NULL,		/* line CA1 in */
	DEVCB_NULL,		/* line CB1 in */
	DEVCB_NULL,		/* line CA2 in */
	DEVCB_NULL,		/* line CB2 in */
	DEVCB_HANDLER(zwackery_pia0_w),		/* port A out */
	DEVCB_NULL,		/* port B out */
	DEVCB_NULL,		/* line CA2 out */
	DEVCB_NULL,		/* port CB2 out */
	DEVCB_LINE(zwackery_pia_irq),		/* IRQA */
	DEVCB_LINE(zwackery_pia_irq)		/* IRQB */
};


const pia6821_interface zwackery_pia1_intf =
{
	DEVCB_HANDLER(zwackery_port_1_r),		/* port A in */
	DEVCB_HANDLER(zwackery_port_2_r),		/* port B in */
	DEVCB_NULL,		/* line CA1 in */
	DEVCB_NULL,		/* line CB1 in */
	DEVCB_NULL,		/* line CA2 in */
	DEVCB_NULL,		/* line CB2 in */
	DEVCB_HANDLER(zwackery_pia1_w),		/* port A out */
	DEVCB_NULL,		/* port B out */
	DEVCB_LINE(zwackery_ca2_w),		/* line CA2 out */
	DEVCB_NULL,		/* port CB2 out */
	DEVCB_NULL,		/* IRQA */
	DEVCB_NULL		/* IRQB */
};


const pia6821_interface zwackery_pia2_intf =
{
	DEVCB_HANDLER(zwackery_port_3_r),		/* port A in */
	DEVCB_INPUT_PORT("DSW"),				/* port B in */
	DEVCB_NULL,		/* line CA1 in */
	DEVCB_NULL,		/* line CB1 in */
	DEVCB_NULL,		/* line CA2 in */
	DEVCB_NULL,		/* line CB2 in */
	DEVCB_NULL,		/* port A out */
	DEVCB_NULL,		/* port B out */
	DEVCB_NULL,		/* line CA2 out */
	DEVCB_NULL,		/* port CB2 out */
	DEVCB_NULL,		/* IRQA */
	DEVCB_NULL		/* IRQB */
};



/*************************************
 *
 *  Generic MCR/68k machine initialization
 *
 *************************************/

MACHINE_START( mcr68 )
{
	int i;

	for (i = 0; i < 3; i++)
	{
		struct counter_state *m6840 = &m6840_state[i];

		m6840->timer = timer_alloc(machine, counter_fired_callback, NULL);

		state_save_register_item(machine, "m6840", NULL, i, m6840->control);
		state_save_register_item(machine, "m6840", NULL, i, m6840->latch);
		state_save_register_item(machine, "m6840", NULL, i, m6840->count);
		state_save_register_item(machine, "m6840", NULL, i, m6840->timer_active);
	}

	state_save_register_global(machine, m6840_status);
	state_save_register_global(machine, m6840_status_read_since_int);
	state_save_register_global(machine, m6840_msb_buffer);
	state_save_register_global(machine, m6840_lsb_buffer);
	state_save_register_global(machine, m6840_irq_state);
	state_save_register_global(machine, v493_irq_state);
	state_save_register_global(machine, zwackery_sound_data);
}


static void mcr68_common_init(running_machine *machine)
{
	int i;

	/* reset the 6840's */
	m6840_counter_periods[0] = ATTOTIME_IN_HZ(30);			/* clocked by /VBLANK */
	m6840_counter_periods[1] = attotime_never;					/* grounded */
	m6840_counter_periods[2] = ATTOTIME_IN_HZ(512 * 30);	/* clocked by /HSYNC */

	m6840_status = 0x00;
	m6840_status_read_since_int = 0x00;
	m6840_msb_buffer = m6840_lsb_buffer = 0;
	for (i = 0; i < 3; i++)
	{
		struct counter_state *m6840 = &m6840_state[i];

		m6840->control = 0x00;
		m6840->latch = 0xffff;
		m6840->count = 0xffff;
		timer_enable(m6840->timer, FALSE);
		m6840->timer_active = 0;
		m6840->period = m6840_counter_periods[i];
	}

	/* initialize the clock */
	m6840_internal_counter_period = ATTOTIME_IN_HZ(cputag_get_clock(machine, "maincpu") / 10);

	/* initialize the sound */
	mcr_sound_reset(machine);
}


MACHINE_RESET( mcr68 )
{
	/* for the most part all MCR/68k games are the same */
	mcr68_common_init(machine);
	v493_callback = mcr68_493_callback;

	/* vectors are 1 and 2 */
	v493_irq_vector = 1;
	m6840_irq_vector = 2;
}


MACHINE_START( zwackery )
{
	MACHINE_START_CALL(mcr68);
}


MACHINE_RESET( zwackery )
{
	/* for the most part all MCR/68k games are the same */
	mcr68_common_init(machine);
	v493_callback = zwackery_493_callback;

	/* vectors are 5 and 6 */
	v493_irq_vector = 5;
	m6840_irq_vector = 6;
}



/*************************************
 *
 *  Generic MCR interrupt handler
 *
 *************************************/

INTERRUPT_GEN( mcr68_interrupt )
{
	/* update the 6840 VBLANK clock */
	if (!m6840_state[0].timer_active)
		subtract_from_counter(device->machine, 0, 1);

	logerror("--- VBLANK ---\n");

	/* also set a timer to generate the 493 signal at a specific time before the next VBLANK */
	/* the timing of this is crucial for Blasted and Tri-Sports, which check the timing of */
	/* VBLANK and 493 using counter 2 */
	timer_set(device->machine, attotime_sub(ATTOTIME_IN_HZ(30), mcr68_timing_factor), NULL, 0, v493_callback);
}



/*************************************
 *
 *  MCR/68k interrupt central
 *
 *************************************/

static void update_mcr68_interrupts(running_machine *machine)
{
	cputag_set_input_line(machine, "maincpu", v493_irq_vector, v493_irq_state ? ASSERT_LINE : CLEAR_LINE);
	cputag_set_input_line(machine, "maincpu", m6840_irq_vector, m6840_irq_state ? ASSERT_LINE : CLEAR_LINE);
}


static TIMER_CALLBACK( mcr68_493_off_callback )
{
	v493_irq_state = 0;
	update_mcr68_interrupts(machine);
}


static TIMER_CALLBACK( mcr68_493_callback )
{
	v493_irq_state = 1;
	update_mcr68_interrupts(machine);
	timer_set(machine, machine->primary_screen->scan_period(), NULL, 0, mcr68_493_off_callback);
	logerror("--- (INT1) ---\n");
}



/*************************************
 *
 *  Zwackery-specific interfaces
 *
 *************************************/

WRITE8_DEVICE_HANDLER( zwackery_pia0_w )
{
	/* bit 7 is the watchdog */
	if (!(data & 0x80)) watchdog_reset(device->machine);

	/* bits 5 and 6 control hflip/vflip */
	/* bits 3 and 4 control coin counters? */
	/* bits 0, 1 and 2 control meters? */
}


WRITE8_DEVICE_HANDLER( zwackery_pia1_w )
{
	zwackery_sound_data = (data >> 4) & 0x0f;
}


WRITE_LINE_DEVICE_HANDLER( zwackery_ca2_w )
{
	address_space *space = cputag_get_address_space(device->machine, "maincpu", ADDRESS_SPACE_PROGRAM);
	csdeluxe_data_w(space, 0, (state << 4) | zwackery_sound_data);
}


static WRITE_LINE_DEVICE_HANDLER( zwackery_pia_irq )
{
	v493_irq_state = pia6821_get_irq_a(device) | pia6821_get_irq_b(device);
	update_mcr68_interrupts(device->machine);
}


static TIMER_CALLBACK( zwackery_493_off_callback )
{
	running_device *pia = machine->device("pia0");
	pia6821_ca1_w(pia, 0);
}


static TIMER_CALLBACK( zwackery_493_callback )
{
	running_device *pia = machine->device("pia0");

	pia6821_ca1_w(pia, 1);
	timer_set(machine, machine->primary_screen->scan_period(), NULL, 0, zwackery_493_off_callback);
}



/*************************************
 *
 *  M6840 timer utilities
 *
 *************************************/

INLINE void update_interrupts(running_machine *machine)
{
	m6840_status &= ~0x80;

	if ((m6840_status & 0x01) && (m6840_state[0].control & 0x40)) m6840_status |= 0x80;
	if ((m6840_status & 0x02) && (m6840_state[1].control & 0x40)) m6840_status |= 0x80;
	if ((m6840_status & 0x04) && (m6840_state[2].control & 0x40)) m6840_status |= 0x80;

	m6840_irq_state = m6840_status >> 7;
	update_mcr68_interrupts(machine);
}


static void subtract_from_counter(running_machine *machine, int counter, int count)
{
	/* dual-byte mode */
	if (m6840_state[counter].control & 0x04)
	{
		int lsb = m6840_state[counter].count & 0xff;
		int msb = m6840_state[counter].count >> 8;

		/* count the clocks */
		lsb -= count;

		/* loop while we're less than zero */
		while (lsb < 0)
		{
			/* borrow from the MSB */
			lsb += (m6840_state[counter].latch & 0xff) + 1;
			msb--;

			/* if MSB goes less than zero, we've expired */
			if (msb < 0)
			{
				m6840_status |= 1 << counter;
				m6840_status_read_since_int &= ~(1 << counter);
				update_interrupts(machine);
				msb = (m6840_state[counter].latch >> 8) + 1;
				LOG(("** Counter %d fired\n", counter));
			}
		}

		/* store the result */
		m6840_state[counter].count = (msb << 8) | lsb;
	}

	/* word mode */
	else
	{
		int word = m6840_state[counter].count;

		/* count the clocks */
		word -= count;

		/* loop while we're less than zero */
		while (word < 0)
		{
			/* borrow from the MSB */
			word += m6840_state[counter].latch + 1;

			/* we've expired */
			m6840_status |= 1 << counter;
			m6840_status_read_since_int &= ~(1 << counter);
			update_interrupts(machine);
			LOG(("** Counter %d fired\n", counter));
		}

		/* store the result */
		m6840_state[counter].count = word;
	}
}


static TIMER_CALLBACK( counter_fired_callback )
{
	int count = param >> 2;
	int counter = param & 3;

	/* reset the timer */
	m6840_state[counter].timer_active = 0;

	/* subtract it all from the counter; this will generate an interrupt */
	subtract_from_counter(machine, counter, count);
}


static void reload_count(int counter)
{
	attotime period;
	attotime total_period;
	int count;

	/* copy the latched value in */
	m6840_state[counter].count = m6840_state[counter].latch;

	/* counter 0 is self-updating if clocked externally */
	if (counter == 0 && !(m6840_state[counter].control & 0x02))
	{
		timer_adjust_oneshot(m6840_state[counter].timer, attotime_never, 0);
		m6840_state[counter].timer_active = 0;
		return;
	}

	/* determine the clock period for this timer */
	if (m6840_state[counter].control & 0x02)
		period = m6840_internal_counter_period;
	else
		period = m6840_counter_periods[counter];

	/* determine the number of clock periods before we expire */
	count = m6840_state[counter].count;
	if (m6840_state[counter].control & 0x04)
		count = ((count >> 8) + 1) * ((count & 0xff) + 1);
	else
		count = count + 1;

	/* set the timer */
	total_period = attotime_make(0, attotime_to_attoseconds(period) * count);
LOG(("reload_count(%d): period = %f  count = %d\n", counter, attotime_to_double(period), count));
	timer_adjust_oneshot(m6840_state[counter].timer, total_period, (count << 2) + counter);
	m6840_state[counter].timer_active = 1;
}


static UINT16 compute_counter(int counter)
{
	attotime period;
	int remaining;

	/* if there's no timer, return the count */
	if (!m6840_state[counter].timer_active)
		return m6840_state[counter].count;

	/* determine the clock period for this timer */
	if (m6840_state[counter].control & 0x02)
		period = m6840_internal_counter_period;
	else
		period = m6840_counter_periods[counter];

	/* see how many are left */
	remaining = attotime_to_attoseconds(timer_timeleft(m6840_state[counter].timer)) / attotime_to_attoseconds(period);

	/* adjust the count for dual byte mode */
	if (m6840_state[counter].control & 0x04)
	{
		int divisor = (m6840_state[counter].count & 0xff) + 1;
		int msb = remaining / divisor;
		int lsb = remaining % divisor;
		remaining = (msb << 8) | lsb;
	}

	return remaining;
}



/*************************************
 *
 *  M6840 timer I/O
 *
 *************************************/

static WRITE8_HANDLER( mcr68_6840_w_common )
{
	int i;

	/* offsets 0 and 1 are control registers */
	if (offset < 2)
	{
		int counter = (offset == 1) ? 1 : (m6840_state[1].control & 0x01) ? 0 : 2;
		UINT8 diffs = data ^ m6840_state[counter].control;

		m6840_state[counter].control = data;

		/* reset? */
		if (counter == 0 && (diffs & 0x01))
		{
			/* holding reset down */
			if (data & 0x01)
			{
				for (i = 0; i < 3; i++)
				{
					timer_adjust_oneshot(m6840_state[i].timer, attotime_never, 0);
					m6840_state[i].timer_active = 0;
				}
			}

			/* releasing reset */
			else
			{
				for (i = 0; i < 3; i++)
					reload_count(i);
			}

			m6840_status = 0;
			update_interrupts(space->machine);
		}

		/* changing the clock source? (needed for Zwackery) */
		if (diffs & 0x02)
			reload_count(counter);

		LOG(("%06X:Counter %d control = %02X\n", cpu_get_previouspc(space->cpu), counter, data));
	}

	/* offsets 2, 4, and 6 are MSB buffer registers */
	else if ((offset & 1) == 0)
	{
		LOG(("%06X:MSB = %02X\n", cpu_get_previouspc(space->cpu), data));
		m6840_msb_buffer = data;
	}

	/* offsets 3, 5, and 7 are Write Timer Latch commands */
	else
	{
		int counter = (offset - 2) / 2;
		m6840_state[counter].latch = (m6840_msb_buffer << 8) | (data & 0xff);

		/* clear the interrupt */
		m6840_status &= ~(1 << counter);
		update_interrupts(space->machine);

		/* reload the count if in an appropriate mode */
		if (!(m6840_state[counter].control & 0x10))
			reload_count(counter);

		LOG(("%06X:Counter %d latch = %04X\n", cpu_get_previouspc(space->cpu), counter, m6840_state[counter].latch));
	}
}


static READ16_HANDLER( mcr68_6840_r_common )
{
	/* offset 0 is a no-op */
	if (offset == 0)
		return 0;

	/* offset 1 is the status register */
	else if (offset == 1)
	{
		LOG(("%06X:Status read = %04X\n", cpu_get_previouspc(space->cpu), m6840_status));
		m6840_status_read_since_int |= m6840_status & 0x07;
		return m6840_status;
	}

	/* offsets 2, 4, and 6 are Read Timer Counter commands */
	else if ((offset & 1) == 0)
	{
		int counter = (offset - 2) / 2;
		int result = compute_counter(counter);

		/* clear the interrupt if the status has been read */
		if (m6840_status_read_since_int & (1 << counter))
			m6840_status &= ~(1 << counter);
		update_interrupts(space->machine);

		m6840_lsb_buffer = result & 0xff;

		LOG(("%06X:Counter %d read = %04X\n", cpu_get_previouspc(space->cpu), counter, result));
		return result >> 8;
	}

	/* offsets 3, 5, and 7 are LSB buffer registers */
	else
		return m6840_lsb_buffer;
}


WRITE16_HANDLER( mcr68_6840_upper_w )
{
	if (ACCESSING_BITS_8_15)
		mcr68_6840_w_common(space, offset, (data >> 8) & 0xff);
}


WRITE16_HANDLER( mcr68_6840_lower_w )
{
	if (ACCESSING_BITS_0_7)
		mcr68_6840_w_common(space, offset, data & 0xff);
}


READ16_HANDLER( mcr68_6840_upper_r )
{
	return (mcr68_6840_r_common(space,offset,0) << 8) | 0x00ff;
}


READ16_HANDLER( mcr68_6840_lower_r )
{
	return mcr68_6840_r_common(space,offset,0) | 0xff00;
}

