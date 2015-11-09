// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    Midway MCR system

***************************************************************************/

#include "emu.h"
#include "audio/midway.h"
#include "includes/mcr.h"
#include "includes/mcr68.h"

#define VERBOSE 0
#define LOG(x) do { if (VERBOSE) logerror x; } while (0)

/*************************************
 *
 *  6821 PIA declarations
 *
 *************************************/

READ8_MEMBER(mcr68_state::zwackery_port_1_r)
{
	UINT8 ret = ioport("IN1")->read();

	downcast<pia6821_device *>(machine().device("pia1"))->set_port_a_z_mask(ret);

	return ret;
}


READ8_MEMBER(mcr68_state::zwackery_port_3_r)
{
	UINT8 ret = ioport("IN3")->read();

	downcast<pia6821_device *>(machine().device("pia2"))->set_port_a_z_mask(ret);

	return ret;
}



/*************************************
 *
 *  Generic MCR/68k machine initialization
 *
 *************************************/

MACHINE_START_MEMBER(mcr68_state,mcr68)
{
	int i;

	for (i = 0; i < 3; i++)
	{
		struct counter_state *m6840 = &m_m6840_state[i];

		m6840->timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(mcr68_state::counter_fired_callback),this));

		save_item(m6840->control, "m6840/control", i);
		save_item(m6840->latch, "m6840/latch", i);
		save_item(m6840->count, "m6840/count", i);
		save_item(m6840->timer_active, "m6840/timer_active", i);
	}

	save_item(NAME(m_m6840_status));
	save_item(NAME(m_m6840_status_read_since_int));
	save_item(NAME(m_m6840_msb_buffer));
	save_item(NAME(m_m6840_lsb_buffer));
	save_item(NAME(m_m6840_irq_state));
	save_item(NAME(m_v493_irq_state));
	save_item(NAME(m_zwackery_sound_data));
}


void mcr68_state::mcr68_common_init()
{
	int i;

	/* reset the 6840's */
	m_m6840_counter_periods[0] = attotime::from_hz(30);          /* clocked by /VBLANK */
	m_m6840_counter_periods[1] = attotime::never;                    /* grounded */
	m_m6840_counter_periods[2] = attotime::from_hz(512 * 30);    /* clocked by /HSYNC */

	m_m6840_status = 0x00;
	m_m6840_status_read_since_int = 0x00;
	m_m6840_msb_buffer = m_m6840_lsb_buffer = 0;
	for (i = 0; i < 3; i++)
	{
		struct counter_state *m6840 = &m_m6840_state[i];

		m6840->control = 0x00;
		m6840->latch = 0xffff;
		m6840->count = 0xffff;
		m6840->timer->enable(false);
		m6840->timer_active = 0;
		m6840->period = m_m6840_counter_periods[i];
	}

	/* initialize the clock */
	m_m6840_internal_counter_period = attotime::from_hz(m_maincpu->unscaled_clock() / 10);
}


MACHINE_RESET_MEMBER(mcr68_state,mcr68)
{
	/* for the most part all MCR/68k games are the same */
	mcr68_common_init();
	m_v493_callback = timer_expired_delegate(FUNC(mcr68_state::mcr68_493_callback),this);

	/* vectors are 1 and 2 */
	m_v493_irq_vector = 1;
	m_m6840_irq_vector = 2;
}


MACHINE_START_MEMBER(mcr68_state,zwackery)
{
	MACHINE_START_CALL_MEMBER(mcr68);
}


MACHINE_RESET_MEMBER(mcr68_state,zwackery)
{
	/* for the most part all MCR/68k games are the same */
	mcr68_common_init();
	m_v493_callback = timer_expired_delegate(FUNC(mcr68_state::zwackery_493_callback),this);

	/* vectors are 5 and 6 */
	m_v493_irq_vector = 5;
	m_m6840_irq_vector = 6;
}



/*************************************
 *
 *  Generic MCR interrupt handler
 *
 *************************************/

INTERRUPT_GEN_MEMBER(mcr68_state::mcr68_interrupt)
{
	/* update the 6840 VBLANK clock */
	if (!m_m6840_state[0].timer_active)
		subtract_from_counter(0, 1);

	logerror("--- VBLANK ---\n");

	/* also set a timer to generate the 493 signal at a specific time before the next VBLANK */
	/* the timing of this is crucial for Blasted and Tri-Sports, which check the timing of */
	/* VBLANK and 493 using counter 2 */
	machine().scheduler().timer_set(attotime::from_hz(30) - m_timing_factor, m_v493_callback);
}



/*************************************
 *
 *  MCR/68k interrupt central
 *
 *************************************/

void mcr68_state::update_mcr68_interrupts()
{
	m_maincpu->set_input_line(m_v493_irq_vector, m_v493_irq_state ? ASSERT_LINE : CLEAR_LINE);
	m_maincpu->set_input_line(m_m6840_irq_vector, m_m6840_irq_state ? ASSERT_LINE : CLEAR_LINE);
}


TIMER_CALLBACK_MEMBER(mcr68_state::mcr68_493_off_callback)
{
	m_v493_irq_state = 0;
	update_mcr68_interrupts();
}


TIMER_CALLBACK_MEMBER(mcr68_state::mcr68_493_callback)
{
	m_v493_irq_state = 1;
	update_mcr68_interrupts();
	machine().scheduler().timer_set(m_screen->scan_period(), timer_expired_delegate(FUNC(mcr68_state::mcr68_493_off_callback),this));
	logerror("--- (INT1) ---\n");
}



/*************************************
 *
 *  Zwackery-specific interfaces
 *
 *************************************/

WRITE8_MEMBER(mcr68_state::zwackery_pia0_w)
{
	/* bit 7 is the watchdog */
	if (!(data & 0x80)) machine().watchdog_reset();

	/* bits 5 and 6 control hflip/vflip */
	/* bits 3 and 4 control coin counters? */
	/* bits 0, 1 and 2 control meters? */
}


WRITE8_MEMBER(mcr68_state::zwackery_pia1_w)
{
	m_zwackery_sound_data = (data >> 4) & 0x0f;
}


WRITE_LINE_MEMBER(mcr68_state::zwackery_ca2_w)
{
	address_space &space = m_maincpu->space(AS_PROGRAM);
	m_chip_squeak_deluxe->write(space, 0, (state << 4) | m_zwackery_sound_data);
}


WRITE_LINE_MEMBER(mcr68_state::zwackery_pia_irq)
{
	pia6821_device *pia = machine().device<pia6821_device>("pia0");
	m_v493_irq_state = pia->irq_a_state() | pia->irq_b_state();
	update_mcr68_interrupts();
}


TIMER_CALLBACK_MEMBER(mcr68_state::zwackery_493_off_callback)
{
	pia6821_device *pia = machine().device<pia6821_device>("pia0");
	pia->ca1_w(0);
}


TIMER_CALLBACK_MEMBER(mcr68_state::zwackery_493_callback)
{
	pia6821_device *pia = machine().device<pia6821_device>("pia0");

	pia->ca1_w(1);
	machine().scheduler().timer_set(m_screen->scan_period(), timer_expired_delegate(FUNC(mcr68_state::zwackery_493_off_callback),this));
}



/*************************************
 *
 *  M6840 timer utilities
 *
 *************************************/

inline void mcr68_state::update_interrupts()
{
	m_m6840_status &= ~0x80;

	if ((m_m6840_status & 0x01) && (m_m6840_state[0].control & 0x40)) m_m6840_status |= 0x80;
	if ((m_m6840_status & 0x02) && (m_m6840_state[1].control & 0x40)) m_m6840_status |= 0x80;
	if ((m_m6840_status & 0x04) && (m_m6840_state[2].control & 0x40)) m_m6840_status |= 0x80;

	m_m6840_irq_state = m_m6840_status >> 7;
	update_mcr68_interrupts();
}


void mcr68_state::subtract_from_counter(int counter, int count)
{
	struct counter_state *m6840 = &m_m6840_state[counter];

	/* dual-byte mode */
	if (m6840->control & 0x04)
	{
		int lsb = m6840->count & 0xff;
		int msb = m6840->count >> 8;

		/* count the clocks */
		lsb -= count;

		/* loop while we're less than zero */
		while (lsb < 0)
		{
			/* borrow from the MSB */
			lsb += (m6840->latch & 0xff) + 1;
			msb--;

			/* if MSB goes less than zero, we've expired */
			if (msb < 0)
			{
				m_m6840_status |= 1 << counter;
				m_m6840_status_read_since_int &= ~(1 << counter);
				update_interrupts();
				msb = (m6840->latch >> 8) + 1;
				LOG(("** Counter %d fired\n", counter));
			}
		}

		/* store the result */
		m6840->count = (msb << 8) | lsb;
	}

	/* word mode */
	else
	{
		int word = m6840->count;

		/* count the clocks */
		word -= count;

		/* loop while we're less than zero */
		while (word < 0)
		{
			/* borrow from the MSB */
			word += m6840->latch + 1;

			/* we've expired */
			m_m6840_status |= 1 << counter;
			m_m6840_status_read_since_int &= ~(1 << counter);
			update_interrupts();
			LOG(("** Counter %d fired\n", counter));
		}

		/* store the result */
		m6840->count = word;
	}
}


TIMER_CALLBACK_MEMBER(mcr68_state::counter_fired_callback)
{
	int count = param >> 2;
	int counter = param & 3;
	struct counter_state *m6840 = &m_m6840_state[counter];

	/* reset the timer */
	m6840->timer_active = 0;

	/* subtract it all from the counter; this will generate an interrupt */
	subtract_from_counter(counter, count);
}


void mcr68_state::reload_count(int counter)
{
	struct counter_state *m6840 = &m_m6840_state[counter];
	attotime period;
	attotime total_period;
	int count;

	/* copy the latched value in */
	m6840->count = m6840->latch;

	/* counter 0 is self-updating if clocked externally */
	if (counter == 0 && !(m6840->control & 0x02))
	{
		m6840->timer->adjust(attotime::never);
		m6840->timer_active = 0;
		return;
	}

	/* determine the clock period for this timer */
	if (m6840->control & 0x02)
		period = m_m6840_internal_counter_period;
	else
		period = m_m6840_counter_periods[counter];

	/* determine the number of clock periods before we expire */
	count = m6840->count;
	if (m6840->control & 0x04)
		count = ((count >> 8) + 1) * ((count & 0xff) + 1);
	else
		count = count + 1;

	/* set the timer */
	total_period = period * count;
LOG(("reload_count(%d): period = %f  count = %d\n", counter, period.as_double(), count));
	m6840->timer->adjust(total_period, (count << 2) + counter);
	m6840->timer_active = 1;
}


UINT16 mcr68_state::compute_counter(int counter)
{
	struct counter_state *m6840 = &m_m6840_state[counter];
	attotime period;
	int remaining;

	/* if there's no timer, return the count */
	if (!m6840->timer_active)
		return m6840->count;

	/* determine the clock period for this timer */
	if (m6840->control & 0x02)
		period = m_m6840_internal_counter_period;
	else
		period = m_m6840_counter_periods[counter];
	/* see how many are left */
	remaining = m6840->timer->remaining().as_attoseconds() / period.as_attoseconds();

	/* adjust the count for dual byte mode */
	if (m6840->control & 0x04)
	{
		int divisor = (m6840->count & 0xff) + 1;
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

WRITE8_MEMBER(mcr68_state::mcr68_6840_w_common)
{
	int i;

	/* offsets 0 and 1 are control registers */
	if (offset < 2)
	{
		int counter = (offset == 1) ? 1 : (m_m6840_state[1].control & 0x01) ? 0 : 2;
		struct counter_state *m6840 = &m_m6840_state[counter];
		UINT8 diffs = data ^ m6840->control;

		m6840->control = data;

		/* reset? */
		if (counter == 0 && (diffs & 0x01))
		{
			/* holding reset down */
			if (data & 0x01)
			{
				for (i = 0; i < 3; i++)
				{
					m_m6840_state[i].timer->adjust(attotime::never);
					m_m6840_state[i].timer_active = 0;
				}
			}

			/* releasing reset */
			else
			{
				for (i = 0; i < 3; i++)
					reload_count(i);
			}

			m_m6840_status = 0;
			update_interrupts();
		}

		/* changing the clock source? (needed for Zwackery) */
		if (diffs & 0x02)
			reload_count(counter);

		LOG(("%06X:Counter %d control = %02X\n", space.device().safe_pcbase(), counter, data));
	}

	/* offsets 2, 4, and 6 are MSB buffer registers */
	else if ((offset & 1) == 0)
	{
		LOG(("%06X:MSB = %02X\n", space.device().safe_pcbase(), data));
		m_m6840_msb_buffer = data;
	}

	/* offsets 3, 5, and 7 are Write Timer Latch commands */
	else
	{
		int counter = (offset - 2) / 2;
		struct counter_state *m6840 = &m_m6840_state[counter];
		m6840->latch = (m_m6840_msb_buffer << 8) | (data & 0xff);

		/* clear the interrupt */
		m_m6840_status &= ~(1 << counter);
		update_interrupts();

		/* reload the count if in an appropriate mode */
		if (!(m6840->control & 0x10))
			reload_count(counter);

		LOG(("%06X:Counter %d latch = %04X\n", space.device().safe_pcbase(), counter, m6840->latch));
	}
}


READ16_MEMBER(mcr68_state::mcr68_6840_r_common)
{
	/* offset 0 is a no-op */
	if (offset == 0)
		return 0;

	/* offset 1 is the status register */
	else if (offset == 1)
	{
		LOG(("%06X:Status read = %04X\n", space.device().safe_pcbase(), m_m6840_status));
		m_m6840_status_read_since_int |= m_m6840_status & 0x07;
		return m_m6840_status;
	}

	/* offsets 2, 4, and 6 are Read Timer Counter commands */
	else if ((offset & 1) == 0)
	{
		int counter = (offset - 2) / 2;
		int result = compute_counter(counter);

		/* clear the interrupt if the status has been read */
		if (m_m6840_status_read_since_int & (1 << counter))
			m_m6840_status &= ~(1 << counter);
		update_interrupts();

		m_m6840_lsb_buffer = result & 0xff;

		LOG(("%06X:Counter %d read = %04X\n", space.device().safe_pcbase(), counter, result));
		return result >> 8;
	}

	/* offsets 3, 5, and 7 are LSB buffer registers */
	else
		return m_m6840_lsb_buffer;
}


WRITE16_MEMBER(mcr68_state::mcr68_6840_upper_w)
{
	if (ACCESSING_BITS_8_15)
		mcr68_6840_w_common(space, offset, (data >> 8) & 0xff);
}


WRITE16_MEMBER(mcr68_state::mcr68_6840_lower_w)
{
	if (ACCESSING_BITS_0_7)
		mcr68_6840_w_common(space, offset, data & 0xff);
}


READ16_MEMBER(mcr68_state::mcr68_6840_upper_r)
{
	return (mcr68_6840_r_common(space,offset,0) << 8) | 0x00ff;
}


READ16_MEMBER(mcr68_state::mcr68_6840_lower_r)
{
	return mcr68_6840_r_common(space,offset,0) | 0xff00;
}
