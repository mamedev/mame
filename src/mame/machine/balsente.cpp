// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    Bally/Sente SAC-1 system

    driver by Aaron Giles

***************************************************************************/

#include "emu.h"
#include "cpu/m6809/m6809.h"
#include "includes/balsente.h"
#include "sound/cem3394.h"


#define LOG_CEM_WRITES      0

/*************************************
 *
 *  Interrupt handling
 *
 *************************************/

TIMER_CALLBACK_MEMBER(balsente_state::irq_off)
{
	m_maincpu->set_input_line(M6809_IRQ_LINE, CLEAR_LINE);
}


TIMER_DEVICE_CALLBACK_MEMBER(balsente_state::balsente_interrupt_timer)
{
	/* next interrupt after scanline 256 is scanline 64 */
	if (param == 256)
		m_scanline_timer->adjust(m_screen->time_until_pos(64), 64);
	else
		m_scanline_timer->adjust(m_screen->time_until_pos(param + 64), param + 64);

	/* IRQ starts on scanline 0, 64, 128, etc. */
	m_maincpu->set_input_line(M6809_IRQ_LINE, ASSERT_LINE);

	/* it will turn off on the next HBLANK */
	machine().scheduler().timer_set(m_screen->time_until_pos(param, BALSENTE_HBSTART), timer_expired_delegate(FUNC(balsente_state::irq_off),this));

	/* if this is Grudge Match, update the steering */
	if (m_grudge_steering_result & 0x80)
		update_grudge_steering();

	/* if we're a shooter, we do a little more work */
	if (m_shooter)
	{
		UINT8 tempx, tempy;

		/* we latch the beam values on the first interrupt after VBLANK */
		if (param == 64)
		{
			m_shooter_x = ioport("FAKEX")->read();
			m_shooter_y = ioport("FAKEY")->read();
		}

		/* which bits get returned depends on which scanline we're at */
		tempx = m_shooter_x << ((param - 64) / 64);
		tempy = m_shooter_y << ((param - 64) / 64);
		m_nstocker_bits = ((tempx >> 4) & 0x08) | ((tempx >> 1) & 0x04) |
								((tempy >> 6) & 0x02) | ((tempy >> 3) & 0x01);
	}
}


void balsente_state::machine_start()
{
	int i;

	m_cem_device[0] = m_cem1;
	m_cem_device[1] = m_cem2;
	m_cem_device[2] = m_cem3;
	m_cem_device[3] = m_cem4;
	m_cem_device[4] = m_cem5;
	m_cem_device[5] = m_cem6;

	/* create the polynomial tables */
	poly17_init();

	/* register for saving */
	for (i = 0; i < 3; i++)
	{
		save_item(m_counter[i].timer_active, "8253counter[i].timer_active", i);
		save_item(m_counter[i].initial, "8253counter[i].initial", i);
		save_item(m_counter[i].count, "8253counter[i].count", i);
		save_item(m_counter[i].gate, "8253counter[i].gate", i);
		save_item(m_counter[i].out, "8253counter[i].out", i);
		save_item(m_counter[i].mode, "8253counter[i].mode", i);
		save_item(m_counter[i].readbyte, "8253counter[i].readbyte", i);
		save_item(m_counter[i].writebyte, "8253counter[i].writebyte", i);
	}

	save_item(NAME(m_counter_control));
	save_item(NAME(m_counter_0_ff));
	save_item(NAME(m_counter_0_timer_active));

	save_item(NAME(m_analog_input_data));
	save_item(NAME(m_adc_value));

	save_item(NAME(m_dac_value));
	save_item(NAME(m_dac_register));
	save_item(NAME(m_chip_select));

	save_item(NAME(m_m6850_status));
	save_item(NAME(m_m6850_control));
	save_item(NAME(m_m6850_input));
	save_item(NAME(m_m6850_output));
	save_item(NAME(m_m6850_data_ready));

	save_item(NAME(m_m6850_sound_status));
	save_item(NAME(m_m6850_sound_control));
	save_item(NAME(m_m6850_sound_input));
	save_item(NAME(m_m6850_sound_output));

	save_item(NAME(m_noise_position));

	save_item(NAME(m_nstocker_bits));
	save_item(NAME(m_spiker_expand_color));
	save_item(NAME(m_spiker_expand_bgcolor));
	save_item(NAME(m_spiker_expand_bits));
	save_item(NAME(m_grudge_steering_result));
	save_item(NAME(m_grudge_last_steering));
}


void balsente_state::machine_reset()
{
	address_space &space = m_maincpu->space(AS_PROGRAM);
	int numbanks;

	/* reset counters; counter 2's gate is tied high */
	memset(m_counter, 0, sizeof(m_counter));
	m_counter[1].timer = machine().device<timer_device>("8253_1_timer");
	m_counter[2].timer = machine().device<timer_device>("8253_2_timer");
	m_counter[2].gate = 1;

	/* reset the manual counter 0 clock */
	m_counter_control = 0x00;
	m_counter_0_ff = 0;
	m_counter_0_timer_active = 0;

	/* reset the ADC states */
	m_adc_value = 0;

	/* reset the CEM3394 I/O states */
	m_dac_value = 0;
	m_dac_register = 0;
	m_chip_select = 0x3f;

	/* reset game-specific states */
	m_grudge_steering_result = 0;

	/* reset the 6850 chips */
	balsente_m6850_w(space, 0, 3);
	balsente_m6850_sound_w(space, 0, 3);

	/* reset the noise generator */
	memset(m_noise_position, 0, sizeof(m_noise_position));

	/* point the banks to bank 0 */
	numbanks = (memregion("maincpu")->bytes() > 0x40000) ? 16 : 8;
	membank("bank1")->configure_entries(0, numbanks, &memregion("maincpu")->base()[0x10000], 0x6000);
	membank("bank2")->configure_entries(0, numbanks, &memregion("maincpu")->base()[0x12000], 0x6000);
	membank("bank1")->set_entry(0);
	membank("bank2")->set_entry(0);
	m_maincpu->reset();

	/* start a timer to generate interrupts */
	m_scanline_timer->adjust(m_screen->time_until_pos(0));
}



/*************************************
 *
 *  MM5837 noise generator
 *
 *  NOTE: this is stolen straight from
 *          POKEY.c
 *
 *************************************/

void balsente_state::poly17_init()
{
	UINT32 i, x = 0;
	UINT8 *p, *r;

	/* allocate memory */
	p = m_poly17;
	r = m_rand17;

	/* generate the polynomial */
	for (i = 0; i < POLY17_SIZE; i++)
	{
		/* store new values */
		*p++ = x & 1;
		*r++ = x >> 3;

		/* calculate next bit */
		x = ((x << POLY17_SHL) + (x >> POLY17_SHR) + POLY17_ADD) & POLY17_SIZE;
	}
}


inline void balsente_state::noise_gen_chip(int chip, int count, short *buffer)
{
	/* noise generator runs at 100kHz */
	UINT32 step = (100000 << 14) / CEM3394_SAMPLE_RATE;
	UINT32 noise_counter = m_noise_position[chip];

	while (count--)
	{
		*buffer++ = m_poly17[(noise_counter >> 14) & POLY17_SIZE] << 12;
		noise_counter += step;
	}

	/* remember the noise position */
	m_noise_position[chip] = noise_counter;
}

CEM3394_EXT_INPUT(balsente_state::noise_gen_0) { noise_gen_chip(0, count, buffer); }
CEM3394_EXT_INPUT(balsente_state::noise_gen_1) { noise_gen_chip(1, count, buffer); }
CEM3394_EXT_INPUT(balsente_state::noise_gen_2) { noise_gen_chip(2, count, buffer); }
CEM3394_EXT_INPUT(balsente_state::noise_gen_3) { noise_gen_chip(3, count, buffer); }
CEM3394_EXT_INPUT(balsente_state::noise_gen_4) { noise_gen_chip(4, count, buffer); }
CEM3394_EXT_INPUT(balsente_state::noise_gen_5) { noise_gen_chip(5, count, buffer); }


/*************************************
 *
 *  Hardware random numbers
 *
 *************************************/

WRITE8_MEMBER(balsente_state::balsente_random_reset_w)
{
	/* reset random number generator */
}


READ8_MEMBER(balsente_state::balsente_random_num_r)
{
	UINT32 cc;

	/* CPU runs at 1.25MHz, noise source at 100kHz --> multiply by 12.5 */
	cc = m_maincpu->total_cycles();

	/* 12.5 = 8 + 4 + 0.5 */
	cc = (cc << 3) + (cc << 2) + (cc >> 1);
	return m_rand17[cc & POLY17_SIZE];
}



/*************************************
 *
 *  ROM banking
 *
 *************************************/

WRITE8_MEMBER(balsente_state::balsente_rombank_select_w)
{
	/* the bank number comes from bits 4-6 */
	membank("bank1")->set_entry((data >> 4) & 7);
	membank("bank2")->set_entry((data >> 4) & 7);
}


WRITE8_MEMBER(balsente_state::balsente_rombank2_select_w)
{
	/* Night Stocker and Name that Tune only so far.... */
	int bank = data & 7;

	/* top bit controls which half of the ROMs to use (Name that Tune only) */
	if (memregion("maincpu")->bytes() > 0x40000) bank |= (data >> 4) & 8;

	/* when they set the AB bank, it appears as though the CD bank is reset */
	if (data & 0x20)
	{
		membank("bank1")->set_entry(bank);
		membank("bank2")->set_entry(6);
	}

	/* set both banks */
	else
	{
		membank("bank1")->set_entry(bank);
		membank("bank2")->set_entry(bank);
	}
}



/*************************************
 *
 *  Special outputs
 *
 *************************************/

WRITE8_MEMBER(balsente_state::balsente_misc_output_w)
{
	offset = (offset / 4) % 8;
	data >>= 7;

	/* these are generally used to control the various lamps */
	/* special case is offset 7, which recalls the NVRAM data */
	if (offset == 7)
	{
		logerror("nvrecall_w=%d\n", data);
	}
	else
	{
//      output().set_led_value(offset, data);
	}
}



/*************************************
 *
 *  6850 UART communications
 *
 *************************************/

void balsente_state::m6850_update_io()
{
	UINT8 new_state;

	/* sound -> main CPU communications */
	if (!(m_m6850_sound_status & 0x02))
	{
		/* set the overrun bit if the data in the destination hasn't been read yet */
		if (m_m6850_status & 0x01)
			m_m6850_status |= 0x20;

		/* copy the sound's output to our input */
		m_m6850_input = m_m6850_sound_output;

		/* set the receive register full bit */
		m_m6850_status |= 0x01;

		/* set the sound's trasmitter register empty bit */
		m_m6850_sound_status |= 0x02;
	}

	/* main -> sound CPU communications */
	if (m_m6850_data_ready)
	{
		/* set the overrun bit if the data in the destination hasn't been read yet */
		if (m_m6850_sound_status & 0x01)
			m_m6850_sound_status |= 0x20;

		/* copy the main CPU's output to our input */
		m_m6850_sound_input = m_m6850_output;

		/* set the receive register full bit */
		m_m6850_sound_status |= 0x01;

		/* set the main CPU's trasmitter register empty bit */
		m_m6850_status |= 0x02;
		m_m6850_data_ready = 0;
	}

	/* check for reset states */
	if ((m_m6850_control & 3) == 3)
	{
		m_m6850_status = 0x02;
		m_m6850_data_ready = 0;
	}
	if ((m_m6850_sound_control & 3) == 3)
		m_m6850_sound_status = 0x02;

	/* check for transmit/receive IRQs on the main CPU */
	new_state = 0;
	if ((m_m6850_control & 0x80) && (m_m6850_status & 0x21)) new_state = 1;
	if ((m_m6850_control & 0x60) == 0x20 && (m_m6850_status & 0x02)) new_state = 1;

	/* apply the change */
	if (new_state && !(m_m6850_status & 0x80))
	{
		m_maincpu->set_input_line(M6809_FIRQ_LINE, ASSERT_LINE);
		m_m6850_status |= 0x80;
	}
	else if (!new_state && (m_m6850_status & 0x80))
	{
		m_maincpu->set_input_line(M6809_FIRQ_LINE, CLEAR_LINE);
		m_m6850_status &= ~0x80;
	}

	/* check for transmit/receive IRQs on the sound CPU */
	new_state = 0;
	if ((m_m6850_sound_control & 0x80) && (m_m6850_sound_status & 0x21)) new_state = 1;
	if ((m_m6850_sound_control & 0x60) == 0x20 && (m_m6850_sound_status & 0x02)) new_state = 1;
	if (!(m_counter_control & 0x20)) new_state = 0;

	/* apply the change */
	if (new_state && !(m_m6850_sound_status & 0x80))
	{
		m_audiocpu->set_input_line(INPUT_LINE_NMI, ASSERT_LINE);
		m_m6850_sound_status |= 0x80;
	}
	else if (!new_state && (m_m6850_sound_status & 0x80))
	{
		m_audiocpu->set_input_line(INPUT_LINE_NMI, CLEAR_LINE);
		m_m6850_sound_status &= ~0x80;
	}
}



/*************************************
 *
 *  6850 UART (main CPU)
 *
 *************************************/

READ8_MEMBER(balsente_state::balsente_m6850_r)
{
	int result;

	/* status register is at offset 0 */
	if (offset == 0)
	{
		result = m_m6850_status;
	}

	/* input register is at offset 1 */
	else
	{
		result = m_m6850_input;

		/* clear the overrun and receive buffer full bits */
		m_m6850_status &= ~0x21;
		m6850_update_io();
	}

	return result;
}


TIMER_CALLBACK_MEMBER(balsente_state::m6850_data_ready_callback)
{
	/* set the output data byte and indicate that we're ready to go */
	m_m6850_output = param;
	m_m6850_data_ready = 1;
	m6850_update_io();
}


TIMER_CALLBACK_MEMBER(balsente_state::m6850_w_callback)
{
	/* indicate that the transmit buffer is no longer empty and update the I/O state */
	m_m6850_status &= ~0x02;
	m6850_update_io();

	/* set a timer for 500usec later to actually transmit the data */
	/* (this is very important for several games, esp Snacks'n Jaxson) */
	machine().scheduler().timer_set(attotime::from_usec(500), timer_expired_delegate(FUNC(balsente_state::m6850_data_ready_callback),this), param);
}


WRITE8_MEMBER(balsente_state::balsente_m6850_w)
{
	/* control register is at offset 0 */
	if (offset == 0)
	{
		m_m6850_control = data;

		/* re-update since interrupt enables could have been modified */
		m6850_update_io();
	}

	/* output register is at offset 1; set a timer to synchronize the CPUs */
	else
		machine().scheduler().synchronize(timer_expired_delegate(FUNC(balsente_state::m6850_w_callback),this), data);
}



/*************************************
 *
 *  6850 UART (sound CPU)
 *
 *************************************/

READ8_MEMBER(balsente_state::balsente_m6850_sound_r)
{
	int result;

	/* status register is at offset 0 */
	if (offset == 0)
	{
		result = m_m6850_sound_status;
	}

	/* input register is at offset 1 */
	else
	{
		result = m_m6850_sound_input;

		/* clear the overrun and receive buffer full bits */
		m_m6850_sound_status &= ~0x21;
		m6850_update_io();
	}

	return result;
}


WRITE8_MEMBER(balsente_state::balsente_m6850_sound_w)
{
	/* control register is at offset 0 */
	if (offset == 0)
		m_m6850_sound_control = data;

	/* output register is at offset 1 */
	else
	{
		m_m6850_sound_output = data;
		m_m6850_sound_status &= ~0x02;
	}

	/* re-update since interrupt enables could have been modified */
	m6850_update_io();
}



/*************************************
 *
 *  ADC handlers
 *
 *************************************/

INTERRUPT_GEN_MEMBER(balsente_state::balsente_update_analog_inputs)
{
	int i;
	static const char *const analog[] = { "AN0", "AN1", "AN2", "AN3" };

	/* the analog input system helpfully scales the value read by the percentage of time */
	/* into the current frame we are; unfortunately, this is bad for us, since the analog */
	/* ports are read once a frame, just at varying intervals. To get around this, we */
	/* read all the analog inputs at VBLANK time and just return the cached values. */
	for (i = 0; i < 4; i++)
		m_analog_input_data[i] = ioport(analog[i])->read();
}


TIMER_CALLBACK_MEMBER(balsente_state::adc_finished)
{
	int which = param;

	/* analog controls are read in two pieces; the lower port returns the sign */
	/* and the upper port returns the absolute value of the magnitude */
	int val = m_analog_input_data[which / 2] << m_adc_shift;

	/* special case for Stompin'/Shrike Avenger */
	if (m_adc_shift == 32)
	{
		m_adc_value = m_analog_input_data[which];
		return;
	}

	/* push everything out a little bit extra; most games seem to have a dead */
	/* zone in the middle that feels unnatural with the mouse */
	if (val < 0) val -= 8;
	else if (val > 0) val += 8;

	/* clip to 0xff maximum magnitude */
	if (val < -0xff) val = -0xff;
	else if (val > 0xff) val = 0xff;

	/* return the sign */
	if (!(which & 1))
		m_adc_value = (val < 0) ? 0xff : 0x00;

	/* return the magnitude */
	else
		m_adc_value = (val < 0) ? -val : val;
}


READ8_MEMBER(balsente_state::balsente_adc_data_r)
{
	/* just return the last value read */
	return m_adc_value;
}


WRITE8_MEMBER(balsente_state::balsente_adc_select_w)
{
	/* set a timer to go off and read the value after 50us */
	/* it's important that we do this for Mini Golf */
logerror("adc_select %d\n", offset & 7);
	machine().scheduler().timer_set(attotime::from_usec(50), timer_expired_delegate(FUNC(balsente_state::adc_finished),this), offset & 7);
}



/*************************************
 *
 *  8253-5 timer utilities
 *
 *  NOTE: this is far from complete!
 *
 *************************************/

void balsente_state::counter_start(int which)
{
	/* don't start a timer for channel 0; it is clocked manually */
	if (which != 0)
	{
		/* only start a timer if we're gated and there is none already */
		if (m_counter[which].gate && !m_counter[which].timer_active)
		{
			m_counter[which].timer_active = 1;
			m_counter[which].timer->adjust(attotime::from_hz(2000000) * m_counter[which].count, which);
		}
	}
}


void balsente_state::counter_stop( int which)
{
	/* only stop the timer if it exists */
	if (m_counter[which].timer_active)
		m_counter[which].timer->reset();
	m_counter[which].timer_active = 0;
}


void balsente_state::counter_update_count(int which)
{
	/* only update if the timer is running */
	if (m_counter[which].timer_active)
	{
		/* determine how many 2MHz cycles are remaining */
		int count = (m_counter[which].timer->time_left() * 2000000).as_double();
		m_counter[which].count = (count < 0) ? 0 : count;
	}
}



/*************************************
 *
 *  8253-5 timer internals
 *
 *  NOTE: this is far from complete!
 *
 *************************************/

void balsente_state::counter_set_gate(int which, int gate)
{
	int oldgate = m_counter[which].gate;

	/* remember the gate state */
	m_counter[which].gate = gate;

	/* if the counter is being halted, update the count and remove the system timer */
	if (!gate && oldgate)
	{
		counter_update_count(which);
		counter_stop(which);
	}

	/* if the counter is being started, create the timer */
	else if (gate && !oldgate)
	{
		/* mode 1 waits for the gate to trigger the counter */
		if (m_counter[which].mode == 1)
		{
			counter_set_out(which, 0);

			/* add one to the count; technically, OUT goes low on the next clock pulse */
			/* and then starts counting down; it's important that we don't count the first one */
			m_counter[which].count = m_counter[which].initial + 1;
		}

		/* start the counter */
		counter_start(which);
	}
}


void balsente_state::counter_set_out(int which, int out)
{
	/* OUT on counter 2 is hooked to the /INT line on the Z80 */
	if (which == 2)
		m_audiocpu->set_input_line(0, out ? ASSERT_LINE : CLEAR_LINE);

	/* OUT on counter 0 is hooked to the GATE line on counter 1 */
	else if (which == 0)
		counter_set_gate(1, !out);

	/* remember the out state */
	m_counter[which].out = out;
}


TIMER_DEVICE_CALLBACK_MEMBER(balsente_state::balsente_counter_callback)
{
	/* reset the counter and the count */
	m_counter[param].timer_active = 0;
	m_counter[param].count = 0;

	/* set the state of the OUT line */
	/* mode 0 and 1: when firing, transition OUT to high */
	if (m_counter[param].mode == 0 || m_counter[param].mode == 1)
		counter_set_out(param, 1);

	/* no other modes handled currently */
}



/*************************************
 *
 *  8253-5 timer handlers
 *
 *  NOTE: this is far from complete!
 *
 *************************************/

READ8_MEMBER(balsente_state::balsente_counter_8253_r)
{
	int which;

	switch (offset & 3)
	{
		case 0:
		case 1:
		case 2:
			/* warning: assumes LSB/MSB addressing and no latching! */
			which = offset & 3;

			/* update the count */
			counter_update_count(which);

			/* return the LSB */
			if (m_counter[which].readbyte == 0)
			{
				m_counter[which].readbyte = 1;
				return m_counter[which].count & 0xff;
			}

			/* write the MSB and reset the counter */
			else
			{
				m_counter[which].readbyte = 0;
				return (m_counter[which].count >> 8) & 0xff;
			}
	}
	return 0;
}


WRITE8_MEMBER(balsente_state::balsente_counter_8253_w)
{
	int which;

	switch (offset & 3)
	{
		case 0:
		case 1:
		case 2:
			/* warning: assumes LSB/MSB addressing and no latching! */
			which = offset & 3;

			/* if the counter is in mode 0, a write here will reset the OUT state */
			if (m_counter[which].mode == 0)
				counter_set_out(which, 0);

			/* write the LSB */
			if (m_counter[which].writebyte == 0)
			{
				m_counter[which].count = (m_counter[which].count & 0xff00) | (data & 0x00ff);
				m_counter[which].initial = (m_counter[which].initial & 0xff00) | (data & 0x00ff);
				m_counter[which].writebyte = 1;
			}

			/* write the MSB and reset the counter */
			else
			{
				m_counter[which].count = (m_counter[which].count & 0x00ff) | ((data << 8) & 0xff00);
				m_counter[which].initial = (m_counter[which].initial & 0x00ff) | ((data << 8) & 0xff00);
				m_counter[which].writebyte = 0;

				/* treat 0 as $10000 */
				if (m_counter[which].count == 0) m_counter[which].count = m_counter[which].initial = 0x10000;

				/* remove any old timer and set a new one */
				counter_stop(which);

				/* note that in mode 1, we have to wait for a rising edge of a gate */
				if (m_counter[which].mode == 0)
					counter_start(which);

				/* if the counter is in mode 1, a write here will set the OUT state */
				if (m_counter[which].mode == 1)
					counter_set_out(which, 1);
			}
			break;

		case 3:
			/* determine which counter */
			which = data >> 6;
			if (which == 3) break;

			/* if the counter was in mode 0, a write here will reset the OUT state */
			if (((m_counter[which].mode >> 1) & 7) == 0)
				counter_set_out(which, 0);

			/* set the mode */
			m_counter[which].mode = (data >> 1) & 7;

			/* if the counter is in mode 0, a write here will reset the OUT state */
			if (m_counter[which].mode == 0)
				counter_set_out(which, 0);
			break;
	}
}



/*************************************
 *
 *  Sound CPU counter 0 emulation
 *
 *************************************/

void balsente_state::set_counter_0_ff(timer_device &timer, int newstate)
{
	/* the flip/flop output is inverted, so if we went high to low, that's a clock */
	if (m_counter_0_ff && !newstate)
	{
		/* only count if gated and non-zero */
		if (m_counter[0].count > 0 && m_counter[0].gate)
		{
			m_counter[0].count--;
			if (m_counter[0].count == 0)
				balsente_counter_callback(timer, nullptr, 0);
		}
	}

	/* remember the new state */
	m_counter_0_ff = newstate;
}


TIMER_DEVICE_CALLBACK_MEMBER(balsente_state::balsente_clock_counter_0_ff)
{
	/* clock the D value through the flip-flop */
	set_counter_0_ff(timer, (m_counter_control >> 3) & 1);
}


void balsente_state::update_counter_0_timer()
{
	double maxfreq = 0.0;
	int i;

	/* if there's already a timer, remove it */
	if (m_counter_0_timer_active)
		m_counter_0_timer->reset();
	m_counter_0_timer_active = 0;

	/* find the counter with the maximum frequency */
	/* this is used to calibrate the timers at startup */
	for (i = 0; i < 6; i++)
		if (m_cem_device[i]->get_parameter(CEM3394_FINAL_GAIN) < 10.0)
		{
			double tempfreq;

			/* if the filter resonance is high, then they're calibrating the filter frequency */
			if (m_cem_device[i]->get_parameter(CEM3394_FILTER_RESONANCE) > 0.9)
				tempfreq = m_cem_device[i]->get_parameter(CEM3394_FILTER_FREQENCY);

			/* otherwise, they're calibrating the VCO frequency */
			else
				tempfreq = m_cem_device[i]->get_parameter(CEM3394_VCO_FREQUENCY);

			if (tempfreq > maxfreq) maxfreq = tempfreq;
		}

	/* reprime the timer */
	if (maxfreq > 0.0)
	{
		m_counter_0_timer_active = 1;
		m_counter_0_timer->adjust(attotime::from_hz(maxfreq), 0, attotime::from_hz(maxfreq));
	}
}



/*************************************
 *
 *  Sound CPU counter handlers
 *
 *************************************/

READ8_MEMBER(balsente_state::balsente_counter_state_r)
{
	/* bit D0 is the inverse of the flip-flop state */
	int result = !m_counter_0_ff;

	/* bit D1 is the OUT value from counter 0 */
	if (m_counter[0].out) result |= 0x02;

	return result;
}


WRITE8_MEMBER(balsente_state::balsente_counter_control_w)
{
	UINT8 diff_counter_control = m_counter_control ^ data;

	/* set the new global value */
	m_counter_control = data;

	/* bit D0 enables/disables audio */
	if (diff_counter_control & 0x01)
	{
		for (auto & elem : m_cem_device)
			elem->set_output_gain(0, (data & 0x01) ? 1.0 : 0);
	}

	/* bit D1 is hooked to counter 0's gate */
	/* if we gate on, start a pulsing timer to clock it */
	if (!m_counter[0].gate && (data & 0x02) && !m_counter_0_timer_active)
	{
		update_counter_0_timer();
	}

	/* if we gate off, remove the timer */
	else if (m_counter[0].gate && !(data & 0x02) && m_counter_0_timer_active)
	{
		m_counter_0_timer->reset();
		m_counter_0_timer_active = 0;
	}

	/* set the actual gate afterwards, since we need to know the old value above */
	counter_set_gate(0, (data >> 1) & 1);

	/* bits D2 and D4 control the clear/reset flags on the flip-flop that feeds counter 0 */
	if (!(data & 0x04)) set_counter_0_ff(*m_counter_0_timer, 1);
	if (!(data & 0x10)) set_counter_0_ff(*m_counter_0_timer, 0);

	/* bit 5 clears the NMI interrupt; recompute the I/O state now */
	m6850_update_io();
}



/*************************************
 *
 *  CEM3394 Interfaces
 *
 *************************************/

WRITE8_MEMBER(balsente_state::balsente_chip_select_w)
{
	static const UINT8 register_map[8] =
	{
		CEM3394_VCO_FREQUENCY,
		CEM3394_FINAL_GAIN,
		CEM3394_FILTER_RESONANCE,
		CEM3394_FILTER_FREQENCY,
		CEM3394_MIXER_BALANCE,
		CEM3394_MODULATION_AMOUNT,
		CEM3394_PULSE_WIDTH,
		CEM3394_WAVE_SELECT
	};

	double voltage = (double)m_dac_value * (8.0 / 4096.0) - 4.0;
	int diffchip = data ^ m_chip_select, i;
	int reg = register_map[m_dac_register];

	/* remember the new select value */
	m_chip_select = data;

	/* check all six chip enables */
	for (i = 0; i < 6; i++)
		if ((diffchip & (1 << i)) && (data & (1 << i)))
		{
#if LOG_CEM_WRITES
			double temp = 0;

			/* remember the previous value */
			temp =
#endif
				m_cem_device[i]->get_parameter(reg);

			/* set the voltage */
			m_cem_device[i]->set_voltage(reg, voltage);

			/* only log changes */
#if LOG_CEM_WRITES
			if (temp != m_cem_device[i]->get_parameter(reg))
			{
				static const char *const names[] =
				{
					"VCO_FREQUENCY",
					"FINAL_GAIN",
					"FILTER_RESONANCE",
					"FILTER_FREQENCY",
					"MIXER_BALANCE",
					"MODULATION_AMOUNT",
					"PULSE_WIDTH",
					"WAVE_SELECT"
				};
				logerror("s%04X:   CEM#%d:%s=%f\n", space.device().safe_pcbase(), i, names[m_dac_register], voltage);
			}
#endif
		}

	/* if a timer for counter 0 is running, recompute */
	if (m_counter_0_timer_active)
		update_counter_0_timer();
}



WRITE8_MEMBER(balsente_state::balsente_dac_data_w)
{
	/* LSB or MSB? */
	if (offset & 1)
		m_dac_value = (m_dac_value & 0xfc0) | ((data >> 2) & 0x03f);
	else
		m_dac_value = (m_dac_value & 0x03f) | ((data << 6) & 0xfc0);

	/* if there are open channels, force the values in */
	if ((m_chip_select & 0x3f) != 0x3f)
	{
		UINT8 temp = m_chip_select;
		balsente_chip_select_w(space, 0, 0x3f);
		balsente_chip_select_w(space, 0, temp);
	}
}


WRITE8_MEMBER(balsente_state::balsente_register_addr_w)
{
	m_dac_register = data & 7;
}



/*************************************
 *
 *  Game-specific handlers
 *
 *************************************/

CUSTOM_INPUT_MEMBER(balsente_state::nstocker_bits_r)
{
	return m_nstocker_bits;
}


WRITE8_MEMBER(balsente_state::spiker_expand_w)
{
	/* offset 0 is the bit pattern */
	if (offset == 0)
		m_spiker_expand_bits = data;

	/* offset 1 is the background color (cleared on each read) */
	else if (offset == 1)
		m_spiker_expand_bgcolor = data;

	/* offset 2 is the color */
	else if (offset == 2)
		m_spiker_expand_color = data;
}


READ8_MEMBER(balsente_state::spiker_expand_r)
{
	UINT8 left, right;

	/* first rotate each nibble */
	m_spiker_expand_bits = ((m_spiker_expand_bits << 1) & 0xee) | ((m_spiker_expand_bits >> 3) & 0x11);

	/* compute left and right pixels */
	left  = (m_spiker_expand_bits & 0x10) ? m_spiker_expand_color : m_spiker_expand_bgcolor;
	right = (m_spiker_expand_bits & 0x01) ? m_spiker_expand_color : m_spiker_expand_bgcolor;

	/* reset the background color */
	m_spiker_expand_bgcolor = 0;

	/* return the combined result */
	return (left & 0xf0) | (right & 0x0f);
}


void balsente_state::update_grudge_steering()
{
	UINT8 wheel[3];
	INT8 diff[3];

	/* read the current steering values */
	wheel[0] = ioport("AN0")->read();
	wheel[1] = ioport("AN1")->read();
	wheel[2] = ioport("AN2")->read();

	/* diff the values */
	diff[0] = wheel[0] - m_grudge_last_steering[0];
	diff[1] = wheel[1] - m_grudge_last_steering[1];
	diff[2] = wheel[2] - m_grudge_last_steering[2];

	/* update the last values */
	m_grudge_last_steering[0] += diff[0];
	m_grudge_last_steering[1] += diff[1];
	m_grudge_last_steering[2] += diff[2];

	/* compute the result */
	m_grudge_steering_result = 0xff;
	if (diff[0])
	{
		m_grudge_steering_result ^= 0x01;
		if (diff[0] > 0) m_grudge_steering_result ^= 0x02;
	}
	if (diff[1])
	{
		m_grudge_steering_result ^= 0x04;
		if (diff[1] > 0) m_grudge_steering_result ^= 0x08;
	}
	if (diff[2])
	{
		m_grudge_steering_result ^= 0x10;
		if (diff[2] > 0) m_grudge_steering_result ^= 0x20;
	}
	logerror("Recomputed steering\n");
}


READ8_MEMBER(balsente_state::grudge_steering_r)
{
	logerror("%04X:grudge_steering_r(@%d)\n", space.device().safe_pc(), m_screen->vpos());
	m_grudge_steering_result |= 0x80;
	return m_grudge_steering_result;
}



/*************************************
 *
 *  Shrike Avenger CPU memory handlers
 *
 *************************************/

READ8_MEMBER(balsente_state::shrike_shared_6809_r)
{
	UINT16 mem_mask_int = offset & 1 ? 0xff00 : 0x00ff;

	switch( offset )
	{
		case 6: // return OK for 68k status register until motors hooked up
			return 0;
		default:
			return ( m_shrike_shared[offset >> 1] & ~mem_mask_int ) >> ( mem_mask_int & 8 );
	}
}


WRITE8_MEMBER(balsente_state::shrike_shared_6809_w)
{
	UINT16 mem_mask_int = offset & 1 ? 0xff00 : 0x00ff;
	m_shrike_shared[offset >> 1] = ( m_shrike_shared[offset >> 1] & mem_mask_int ) | ( data << ( mem_mask_int & 0x8 ) );
}

// uses movep, so writes even 8 bit addresses to odd 16 bit addresses, reads as 16 bit from odd addresses
// i.e. write 0xdeadbeef to 10000, read 0xde from 10001, 0xad from 10003, 0xbe from 10005...
WRITE16_MEMBER(balsente_state::shrike_io_68k_w)
{
	COMBINE_DATA( &m_shrike_io[offset] );
}

READ16_MEMBER(balsente_state::shrike_io_68k_r)
{
	return ( m_shrike_io[offset] & mem_mask ) >> ( 8 & ~mem_mask );
}
