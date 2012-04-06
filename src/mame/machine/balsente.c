/***************************************************************************

    Bally/Sente SAC-1 system

    driver by Aaron Giles

***************************************************************************/

#include "emu.h"
#include "cpu/m6809/m6809.h"
#include "includes/balsente.h"
#include "sound/cem3394.h"


#define LOG_CEM_WRITES		0


/* local prototypes */
static void poly17_init(running_machine &machine);
static void update_grudge_steering(running_machine &machine);




/*************************************
 *
 *  Interrupt handling
 *
 *************************************/

static TIMER_CALLBACK( irq_off )
{
	cputag_set_input_line(machine, "maincpu", M6809_IRQ_LINE, CLEAR_LINE);
}


TIMER_DEVICE_CALLBACK( balsente_interrupt_timer )
{
	balsente_state *state = timer.machine().driver_data<balsente_state>();

	/* next interrupt after scanline 256 is scanline 64 */
	if (param == 256)
		state->m_scanline_timer->adjust(timer.machine().primary_screen->time_until_pos(64), 64);
	else
		state->m_scanline_timer->adjust(timer.machine().primary_screen->time_until_pos(param + 64), param + 64);

	/* IRQ starts on scanline 0, 64, 128, etc. */
	cputag_set_input_line(timer.machine(), "maincpu", M6809_IRQ_LINE, ASSERT_LINE);

	/* it will turn off on the next HBLANK */
	timer.machine().scheduler().timer_set(timer.machine().primary_screen->time_until_pos(param, BALSENTE_HBSTART), FUNC(irq_off));

	/* if this is Grudge Match, update the steering */
	if (state->m_grudge_steering_result & 0x80)
		update_grudge_steering(timer.machine());

	/* if we're a shooter, we do a little more work */
	if (state->m_shooter)
	{
		UINT8 tempx, tempy;

		/* we latch the beam values on the first interrupt after VBLANK */
		if (param == 64)
		{
			state->m_shooter_x = input_port_read(timer.machine(), "FAKEX");
			state->m_shooter_y = input_port_read(timer.machine(), "FAKEY");
		}

		/* which bits get returned depends on which scanline we're at */
		tempx = state->m_shooter_x << ((param - 64) / 64);
		tempy = state->m_shooter_y << ((param - 64) / 64);
		state->m_nstocker_bits = ((tempx >> 4) & 0x08) | ((tempx >> 1) & 0x04) |
								((tempy >> 6) & 0x02) | ((tempy >> 3) & 0x01);
	}
}


MACHINE_START( balsente )
{
	balsente_state *state = machine.driver_data<balsente_state>();
	int i;

	state->m_cem_device[0] = state->m_cem1;
	state->m_cem_device[1] = state->m_cem2;
	state->m_cem_device[2] = state->m_cem3;
	state->m_cem_device[3] = state->m_cem4;
	state->m_cem_device[4] = state->m_cem5;
	state->m_cem_device[5] = state->m_cem6;

	/* create the polynomial tables */
	poly17_init(machine);

	/* register for saving */
	for (i = 0; i < 3; i++)
	{
		state_save_register_item(machine, "8253counter", NULL, i, state->m_counter[i].timer_active);
		state_save_register_item(machine, "8253counter", NULL, i, state->m_counter[i].initial);
		state_save_register_item(machine, "8253counter", NULL, i, state->m_counter[i].count);
		state_save_register_item(machine, "8253counter", NULL, i, state->m_counter[i].gate);
		state_save_register_item(machine, "8253counter", NULL, i, state->m_counter[i].out);
		state_save_register_item(machine, "8253counter", NULL, i, state->m_counter[i].mode);
		state_save_register_item(machine, "8253counter", NULL, i, state->m_counter[i].readbyte);
		state_save_register_item(machine, "8253counter", NULL, i, state->m_counter[i].writebyte);
	}

	state->save_item(NAME(state->m_counter_control));
	state->save_item(NAME(state->m_counter_0_ff));
	state->save_item(NAME(state->m_counter_0_timer_active));

	state->save_item(NAME(state->m_analog_input_data));
	state->save_item(NAME(state->m_adc_value));

	state->save_item(NAME(state->m_dac_value));
	state->save_item(NAME(state->m_dac_register));
	state->save_item(NAME(state->m_chip_select));

	state->save_item(NAME(state->m_m6850_status));
	state->save_item(NAME(state->m_m6850_control));
	state->save_item(NAME(state->m_m6850_input));
	state->save_item(NAME(state->m_m6850_output));
	state->save_item(NAME(state->m_m6850_data_ready));

	state->save_item(NAME(state->m_m6850_sound_status));
	state->save_item(NAME(state->m_m6850_sound_control));
	state->save_item(NAME(state->m_m6850_sound_input));
	state->save_item(NAME(state->m_m6850_sound_output));

	state->save_item(NAME(state->m_noise_position));

	state->save_item(NAME(state->m_nstocker_bits));
	state->save_item(NAME(state->m_spiker_expand_color));
	state->save_item(NAME(state->m_spiker_expand_bgcolor));
	state->save_item(NAME(state->m_spiker_expand_bits));
	state->save_item(NAME(state->m_grudge_steering_result));
	state->save_item(NAME(state->m_grudge_last_steering));
}


MACHINE_RESET( balsente )
{
	address_space *space = machine.device("maincpu")->memory().space(AS_PROGRAM);
	balsente_state *state = machine.driver_data<balsente_state>();
	int numbanks;

	/* reset counters; counter 2's gate is tied high */
	memset(state->m_counter, 0, sizeof(state->m_counter));
	state->m_counter[1].timer = machine.device<timer_device>("8253_1_timer");
	state->m_counter[2].timer = machine.device<timer_device>("8253_2_timer");
	state->m_counter[2].gate = 1;

	/* reset the manual counter 0 clock */
	state->m_counter_control = 0x00;
	state->m_counter_0_ff = 0;
	state->m_counter_0_timer_active = 0;

	/* reset the ADC states */
	state->m_adc_value = 0;

	/* reset the CEM3394 I/O states */
	state->m_dac_value = 0;
	state->m_dac_register = 0;
	state->m_chip_select = 0x3f;

	/* reset game-specific states */
	state->m_grudge_steering_result = 0;

	/* reset the 6850 chips */
	state->balsente_m6850_w(*space, 0, 3);
	state->balsente_m6850_sound_w(*space, 0, 3);

	/* reset the noise generator */
	memset(state->m_noise_position, 0, sizeof(state->m_noise_position));

	/* point the banks to bank 0 */
	numbanks = (machine.region("maincpu")->bytes() > 0x40000) ? 16 : 8;
	memory_configure_bank(machine, "bank1", 0, numbanks, &machine.region("maincpu")->base()[0x10000], 0x6000);
	memory_configure_bank(machine, "bank2", 0, numbanks, &machine.region("maincpu")->base()[0x12000], 0x6000);
	memory_set_bank(space->machine(), "bank1", 0);
	memory_set_bank(space->machine(), "bank2", 0);
	machine.device("maincpu")->reset();

	/* start a timer to generate interrupts */
	state->m_scanline_timer->adjust(machine.primary_screen->time_until_pos(0));
}



/*************************************
 *
 *  MM5837 noise generator
 *
 *  NOTE: this is stolen straight from
 *          POKEY.c
 *
 *************************************/

static void poly17_init(running_machine &machine)
{
	balsente_state *state = machine.driver_data<balsente_state>();
	UINT32 i, x = 0;
	UINT8 *p, *r;

	/* allocate memory */
	p = state->m_poly17;
	r = state->m_rand17;

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


void balsente_noise_gen(device_t *device, int count, short *buffer)
{
	balsente_state *state = device->machine().driver_data<balsente_state>();
	int chip;
	UINT32 step, noise_counter;

	/* find the chip we are referring to */
	for (chip = 0; chip < ARRAY_LENGTH(state->m_cem_device); chip++)
		if (device == state->m_cem_device[chip])
			break;
	assert(chip < ARRAY_LENGTH(state->m_cem_device));

	/* noise generator runs at 100kHz */
	step = (100000 << 14) / CEM3394_SAMPLE_RATE;
	noise_counter = state->m_noise_position[chip];

	while (count--)
	{
		*buffer++ = state->m_poly17[(noise_counter >> 14) & POLY17_SIZE] << 12;
		noise_counter += step;
	}

	/* remember the noise position */
	state->m_noise_position[chip] = noise_counter;
}



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
	cc = machine().firstcpu->total_cycles();

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
	memory_set_bank(machine(), "bank1", (data >> 4) & 7);
	memory_set_bank(machine(), "bank2", (data >> 4) & 7);
}


WRITE8_MEMBER(balsente_state::balsente_rombank2_select_w)
{
	/* Night Stocker and Name that Tune only so far.... */
	int bank = data & 7;

	/* top bit controls which half of the ROMs to use (Name that Tune only) */
	if (machine().region("maincpu")->bytes() > 0x40000) bank |= (data >> 4) & 8;

	/* when they set the AB bank, it appears as though the CD bank is reset */
	if (data & 0x20)
	{
		memory_set_bank(machine(), "bank1", bank);
		memory_set_bank(machine(), "bank2", 6);
	}

	/* set both banks */
	else
	{
		memory_set_bank(machine(), "bank1", bank);
		memory_set_bank(machine(), "bank2", bank);
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
//      set_led_status(machine(), offset, data);
	}
}



/*************************************
 *
 *  6850 UART communications
 *
 *************************************/

static void m6850_update_io(running_machine &machine)
{
	balsente_state *state = machine.driver_data<balsente_state>();
	UINT8 new_state;

	/* sound -> main CPU communications */
	if (!(state->m_m6850_sound_status & 0x02))
	{
		/* set the overrun bit if the data in the destination hasn't been read yet */
		if (state->m_m6850_status & 0x01)
			state->m_m6850_status |= 0x20;

		/* copy the sound's output to our input */
		state->m_m6850_input = state->m_m6850_sound_output;

		/* set the receive register full bit */
		state->m_m6850_status |= 0x01;

		/* set the sound's trasmitter register empty bit */
		state->m_m6850_sound_status |= 0x02;
	}

	/* main -> sound CPU communications */
	if (state->m_m6850_data_ready)
	{
		/* set the overrun bit if the data in the destination hasn't been read yet */
		if (state->m_m6850_sound_status & 0x01)
			state->m_m6850_sound_status |= 0x20;

		/* copy the main CPU's output to our input */
		state->m_m6850_sound_input = state->m_m6850_output;

		/* set the receive register full bit */
		state->m_m6850_sound_status |= 0x01;

		/* set the main CPU's trasmitter register empty bit */
		state->m_m6850_status |= 0x02;
		state->m_m6850_data_ready = 0;
	}

	/* check for reset states */
	if ((state->m_m6850_control & 3) == 3)
	{
		state->m_m6850_status = 0x02;
		state->m_m6850_data_ready = 0;
	}
	if ((state->m_m6850_sound_control & 3) == 3)
		state->m_m6850_sound_status = 0x02;

	/* check for transmit/receive IRQs on the main CPU */
	new_state = 0;
	if ((state->m_m6850_control & 0x80) && (state->m_m6850_status & 0x21)) new_state = 1;
	if ((state->m_m6850_control & 0x60) == 0x20 && (state->m_m6850_status & 0x02)) new_state = 1;

	/* apply the change */
	if (new_state && !(state->m_m6850_status & 0x80))
	{
		cputag_set_input_line(machine, "maincpu", M6809_FIRQ_LINE, ASSERT_LINE);
		state->m_m6850_status |= 0x80;
	}
	else if (!new_state && (state->m_m6850_status & 0x80))
	{
		cputag_set_input_line(machine, "maincpu", M6809_FIRQ_LINE, CLEAR_LINE);
		state->m_m6850_status &= ~0x80;
	}

	/* check for transmit/receive IRQs on the sound CPU */
	new_state = 0;
	if ((state->m_m6850_sound_control & 0x80) && (state->m_m6850_sound_status & 0x21)) new_state = 1;
	if ((state->m_m6850_sound_control & 0x60) == 0x20 && (state->m_m6850_sound_status & 0x02)) new_state = 1;
	if (!(state->m_counter_control & 0x20)) new_state = 0;

	/* apply the change */
	if (new_state && !(state->m_m6850_sound_status & 0x80))
	{
		cputag_set_input_line(machine, "audiocpu", INPUT_LINE_NMI, ASSERT_LINE);
		state->m_m6850_sound_status |= 0x80;
	}
	else if (!new_state && (state->m_m6850_sound_status & 0x80))
	{
		cputag_set_input_line(machine, "audiocpu", INPUT_LINE_NMI, CLEAR_LINE);
		state->m_m6850_sound_status &= ~0x80;
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
		m6850_update_io(machine());
	}

	return result;
}


static TIMER_CALLBACK( m6850_data_ready_callback )
{
	balsente_state *state = machine.driver_data<balsente_state>();

	/* set the output data byte and indicate that we're ready to go */
	state->m_m6850_output = param;
	state->m_m6850_data_ready = 1;
	m6850_update_io(machine);
}


static TIMER_CALLBACK( m6850_w_callback )
{
	balsente_state *state = machine.driver_data<balsente_state>();

	/* indicate that the transmit buffer is no longer empty and update the I/O state */
	state->m_m6850_status &= ~0x02;
	m6850_update_io(machine);

	/* set a timer for 500usec later to actually transmit the data */
	/* (this is very important for several games, esp Snacks'n Jaxson) */
	machine.scheduler().timer_set(attotime::from_usec(500), FUNC(m6850_data_ready_callback), param);
}


WRITE8_MEMBER(balsente_state::balsente_m6850_w)
{

	/* control register is at offset 0 */
	if (offset == 0)
	{
		m_m6850_control = data;

		/* re-update since interrupt enables could have been modified */
		m6850_update_io(machine());
	}

	/* output register is at offset 1; set a timer to synchronize the CPUs */
	else
		machine().scheduler().synchronize(FUNC(m6850_w_callback), data);
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
		m6850_update_io(machine());
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
	m6850_update_io(machine());
}



/*************************************
 *
 *  ADC handlers
 *
 *************************************/

INTERRUPT_GEN( balsente_update_analog_inputs )
{
	balsente_state *state = device->machine().driver_data<balsente_state>();
	int i;
	static const char *const analog[] = { "AN0", "AN1", "AN2", "AN3" };

	/* the analog input system helpfully scales the value read by the percentage of time */
	/* into the current frame we are; unfortunately, this is bad for us, since the analog */
	/* ports are read once a frame, just at varying intervals. To get around this, we */
	/* read all the analog inputs at VBLANK time and just return the cached values. */
	for (i = 0; i < 4; i++)
		state->m_analog_input_data[i] = input_port_read(device->machine(), analog[i]);
}


static TIMER_CALLBACK( adc_finished )
{
	balsente_state *state = machine.driver_data<balsente_state>();
	int which = param;

	/* analog controls are read in two pieces; the lower port returns the sign */
	/* and the upper port returns the absolute value of the magnitude */
	int val = state->m_analog_input_data[which / 2] << state->m_adc_shift;

	/* special case for Stompin'/Shrike Avenger */
	if (state->m_adc_shift == 32)
	{
		state->m_adc_value = state->m_analog_input_data[which];
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
		state->m_adc_value = (val < 0) ? 0xff : 0x00;

	/* return the magnitude */
	else
		state->m_adc_value = (val < 0) ? -val : val;
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
	machine().scheduler().timer_set(attotime::from_usec(50), FUNC(adc_finished), offset & 7);
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
		cputag_set_input_line(machine(), "audiocpu", 0, out ? ASSERT_LINE : CLEAR_LINE);

	/* OUT on counter 0 is hooked to the GATE line on counter 1 */
	else if (which == 0)
		counter_set_gate(1, !out);

	/* remember the out state */
	m_counter[which].out = out;
}


TIMER_DEVICE_CALLBACK( balsente_counter_callback )
{
	balsente_state *state = timer.machine().driver_data<balsente_state>();

	/* reset the counter and the count */
	state->m_counter[param].timer_active = 0;
	state->m_counter[param].count = 0;

	/* set the state of the OUT line */
	/* mode 0 and 1: when firing, transition OUT to high */
	if (state->m_counter[param].mode == 0 || state->m_counter[param].mode == 1)
		state->counter_set_out(param, 1);

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
			break;
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

static void set_counter_0_ff(timer_device &timer, int newstate)
{
	balsente_state *state = timer.machine().driver_data<balsente_state>();

	/* the flip/flop output is inverted, so if we went high to low, that's a clock */
	if (state->m_counter_0_ff && !newstate)
	{
		/* only count if gated and non-zero */
		if (state->m_counter[0].count > 0 && state->m_counter[0].gate)
		{
			state->m_counter[0].count--;
			if (state->m_counter[0].count == 0)
				balsente_counter_callback(timer, NULL, 0);
		}
	}

	/* remember the new state */
	state->m_counter_0_ff = newstate;
}


TIMER_DEVICE_CALLBACK( balsente_clock_counter_0_ff )
{
	balsente_state *state = timer.machine().driver_data<balsente_state>();

	/* clock the D value through the flip-flop */
	set_counter_0_ff(timer, (state->m_counter_control >> 3) & 1);
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
		if (cem3394_get_parameter(m_cem_device[i], CEM3394_FINAL_GAIN) < 10.0)
		{
			double tempfreq;

			/* if the filter resonance is high, then they're calibrating the filter frequency */
			if (cem3394_get_parameter(m_cem_device[i], CEM3394_FILTER_RESONANCE) > 0.9)
				tempfreq = cem3394_get_parameter(m_cem_device[i], CEM3394_FILTER_FREQENCY);

			/* otherwise, they're calibrating the VCO frequency */
			else
				tempfreq = cem3394_get_parameter(m_cem_device[i], CEM3394_VCO_FREQUENCY);

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
		int ch;
		for (ch = 0; ch < 6; ch++)
			m_cem_device[ch]->set_output_gain(0, (data & 0x01) ? 1.0 : 0);
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
	m6850_update_io(machine());
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
				cem3394_get_parameter(m_cem_device[i], reg);

			/* set the voltage */
			cem3394_set_voltage(m_cem_device[i], reg, voltage);

			/* only log changes */
#if LOG_CEM_WRITES
			if (temp != cem3394_get_parameter(m_cem_device[i], reg))
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
				logerror("s%04X:   CEM#%d:%s=%f\n", cpu_get_previouspc(&space.device()), i, names[m_dac_register], voltage);
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

CUSTOM_INPUT( nstocker_bits_r )
{
	balsente_state *state = field.machine().driver_data<balsente_state>();
	return state->m_nstocker_bits;
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


static void update_grudge_steering(running_machine &machine)
{
	balsente_state *state = machine.driver_data<balsente_state>();
	UINT8 wheel[3];
	INT8 diff[3];

	/* read the current steering values */
	wheel[0] = input_port_read(machine, "AN0");
	wheel[1] = input_port_read(machine, "AN1");
	wheel[2] = input_port_read(machine, "AN2");

	/* diff the values */
	diff[0] = wheel[0] - state->m_grudge_last_steering[0];
	diff[1] = wheel[1] - state->m_grudge_last_steering[1];
	diff[2] = wheel[2] - state->m_grudge_last_steering[2];

	/* update the last values */
	state->m_grudge_last_steering[0] += diff[0];
	state->m_grudge_last_steering[1] += diff[1];
	state->m_grudge_last_steering[2] += diff[2];

	/* compute the result */
	state->m_grudge_steering_result = 0xff;
	if (diff[0])
	{
		state->m_grudge_steering_result ^= 0x01;
		if (diff[0] > 0) state->m_grudge_steering_result ^= 0x02;
	}
	if (diff[1])
	{
		state->m_grudge_steering_result ^= 0x04;
		if (diff[1] > 0) state->m_grudge_steering_result ^= 0x08;
	}
	if (diff[2])
	{
		state->m_grudge_steering_result ^= 0x10;
		if (diff[2] > 0) state->m_grudge_steering_result ^= 0x20;
	}
	logerror("Recomputed steering\n");
}


READ8_MEMBER(balsente_state::grudge_steering_r)
{
	logerror("%04X:grudge_steering_r(@%d)\n", cpu_get_pc(&space.device()), machine().primary_screen->vpos());
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

