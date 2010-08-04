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
static void poly17_init(running_machine *machine);
static void counter_set_out(running_machine *machine, int which, int gate);
static void update_grudge_steering(running_machine *machine);




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
	balsente_state *state = timer.machine->driver_data<balsente_state>();

	/* next interrupt after scanline 256 is scanline 64 */
	if (param == 256)
		state->scanline_timer->adjust(timer.machine->primary_screen->time_until_pos(64), 64);
	else
		state->scanline_timer->adjust(timer.machine->primary_screen->time_until_pos(param + 64), param + 64);

	/* IRQ starts on scanline 0, 64, 128, etc. */
	cputag_set_input_line(timer.machine, "maincpu", M6809_IRQ_LINE, ASSERT_LINE);

	/* it will turn off on the next HBLANK */
	timer_set(timer.machine, timer.machine->primary_screen->time_until_pos(param, BALSENTE_HBSTART), NULL, 0, irq_off);

	/* if this is Grudge Match, update the steering */
	if (state->grudge_steering_result & 0x80)
		update_grudge_steering(timer.machine);

	/* if we're a shooter, we do a little more work */
	if (state->shooter)
	{
		UINT8 tempx, tempy;

		/* we latch the beam values on the first interrupt after VBLANK */
		if (param == 64)
		{
			state->shooter_x = input_port_read(timer.machine, "FAKEX");
			state->shooter_y = input_port_read(timer.machine, "FAKEY");
		}

		/* which bits get returned depends on which scanline we're at */
		tempx = state->shooter_x << ((param - 64) / 64);
		tempy = state->shooter_y << ((param - 64) / 64);
		state->nstocker_bits = ((tempx >> 4) & 0x08) | ((tempx >> 1) & 0x04) |
								((tempy >> 6) & 0x02) | ((tempy >> 3) & 0x01);
	}
}


MACHINE_START( balsente )
{
	balsente_state *state = machine->driver_data<balsente_state>();
	int i;

	/* create the polynomial tables */
	poly17_init(machine);

	/* register for saving */
	for (i = 0; i < 3; i++)
	{
		state_save_register_item(machine, "8253counter", NULL, i, state->counter[i].timer_active);
		state_save_register_item(machine, "8253counter", NULL, i, state->counter[i].initial);
		state_save_register_item(machine, "8253counter", NULL, i, state->counter[i].count);
		state_save_register_item(machine, "8253counter", NULL, i, state->counter[i].gate);
		state_save_register_item(machine, "8253counter", NULL, i, state->counter[i].out);
		state_save_register_item(machine, "8253counter", NULL, i, state->counter[i].mode);
		state_save_register_item(machine, "8253counter", NULL, i, state->counter[i].readbyte);
		state_save_register_item(machine, "8253counter", NULL, i, state->counter[i].writebyte);
	}

	state_save_register_global(machine, state->counter_control);
	state_save_register_global(machine, state->counter_0_ff);
	state_save_register_global(machine, state->counter_0_timer_active);

	state_save_register_global_array(machine, state->analog_input_data);
	state_save_register_global(machine, state->adc_value);

	state_save_register_global(machine, state->dac_value);
	state_save_register_global(machine, state->dac_register);
	state_save_register_global(machine, state->chip_select);

	state_save_register_global(machine, state->m6850_status);
	state_save_register_global(machine, state->m6850_control);
	state_save_register_global(machine, state->m6850_input);
	state_save_register_global(machine, state->m6850_output);
	state_save_register_global(machine, state->m6850_data_ready);

	state_save_register_global(machine, state->m6850_sound_status);
	state_save_register_global(machine, state->m6850_sound_control);
	state_save_register_global(machine, state->m6850_sound_input);
	state_save_register_global(machine, state->m6850_sound_output);

	state_save_register_global_array(machine, state->noise_position);

	state_save_register_global(machine, state->nstocker_bits);
	state_save_register_global(machine, state->spiker_expand_color);
	state_save_register_global(machine, state->spiker_expand_bgcolor);
	state_save_register_global(machine, state->spiker_expand_bits);
	state_save_register_global(machine, state->grudge_steering_result);
	state_save_register_global_array(machine, state->grudge_last_steering);
}


MACHINE_RESET( balsente )
{
	const address_space *space = cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM);
	balsente_state *state = machine->driver_data<balsente_state>();
	int numbanks;

	/* reset counters; counter 2's gate is tied high */
	memset(state->counter, 0, sizeof(state->counter));
	state->counter[1].timer = machine->device<timer_device>("8253_1_timer");
	state->counter[2].timer = machine->device<timer_device>("8253_2_timer");
	state->counter[2].gate = 1;

	/* reset the manual counter 0 clock */
	state->counter_control = 0x00;
	state->counter_0_ff = 0;
	state->counter_0_timer_active = 0;

	/* reset the ADC states */
	state->adc_value = 0;

	/* reset the CEM3394 I/O states */
	state->dac_value = 0;
	state->dac_register = 0;
	state->chip_select = 0x3f;

	/* reset game-specific states */
	state->grudge_steering_result = 0;

	/* reset the 6850 chips */
	balsente_m6850_w(space, 0, 3);
	balsente_m6850_sound_w(space, 0, 3);

	/* reset the noise generator */
	memset(state->noise_position, 0, sizeof(state->noise_position));

	/* point the banks to bank 0 */
	numbanks = (memory_region_length(machine, "maincpu") > 0x40000) ? 16 : 8;
	memory_configure_bank(machine, "bank1", 0, numbanks, &memory_region(machine, "maincpu")[0x10000], 0x6000);
	memory_configure_bank(machine, "bank2", 0, numbanks, &memory_region(machine, "maincpu")[0x12000], 0x6000);
	memory_set_bank(space->machine, "bank1", 0);
	memory_set_bank(space->machine, "bank2", 0);
	machine->device("maincpu")->reset();

	/* start a timer to generate interrupts */
	state->scanline_timer->adjust(machine->primary_screen->time_until_pos(0));
}



/*************************************
 *
 *  MM5837 noise generator
 *
 *  NOTE: this is stolen straight from
 *          POKEY.c
 *
 *************************************/

static void poly17_init(running_machine *machine)
{
	balsente_state *state = machine->driver_data<balsente_state>();
	UINT32 i, x = 0;
	UINT8 *p, *r;

	/* allocate memory */
	p = state->poly17;
	r = state->rand17;

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


void balsente_noise_gen(running_device *device, int count, short *buffer)
{
	balsente_state *state = device->machine->driver_data<balsente_state>();
	int chip;
	UINT32 step, noise_counter;

	/* find the chip we are referring to */
	for (chip = 0; chip < ARRAY_LENGTH(state->cem_device); chip++)
		if (device == state->cem_device[chip])
			break;
	assert(chip < ARRAY_LENGTH(state->cem_device));

	/* noise generator runs at 100kHz */
	step = (100000 << 14) / CEM3394_SAMPLE_RATE;
	noise_counter = state->noise_position[chip];

	while (count--)
	{
		*buffer++ = state->poly17[(noise_counter >> 14) & POLY17_SIZE] << 12;
		noise_counter += step;
	}

	/* remember the noise position */
	state->noise_position[chip] = noise_counter;
}



/*************************************
 *
 *  Hardware random numbers
 *
 *************************************/

WRITE8_HANDLER( balsente_random_reset_w )
{
	/* reset random number generator */
}


READ8_HANDLER( balsente_random_num_r )
{
	balsente_state *state = space->machine->driver_data<balsente_state>();
	UINT32 cc;

	/* CPU runs at 1.25MHz, noise source at 100kHz --> multiply by 12.5 */
	cc = space->machine->firstcpu->total_cycles();

	/* 12.5 = 8 + 4 + 0.5 */
	cc = (cc << 3) + (cc << 2) + (cc >> 1);
	return state->rand17[cc & POLY17_SIZE];
}



/*************************************
 *
 *  ROM banking
 *
 *************************************/

WRITE8_HANDLER( balsente_rombank_select_w )
{
	/* the bank number comes from bits 4-6 */
	memory_set_bank(space->machine, "bank1", (data >> 4) & 7);
	memory_set_bank(space->machine, "bank2", (data >> 4) & 7);
}


WRITE8_HANDLER( balsente_rombank2_select_w )
{
	/* Night Stocker and Name that Tune only so far.... */
	int bank = data & 7;

	/* top bit controls which half of the ROMs to use (Name that Tune only) */
	if (memory_region_length(space->machine, "maincpu") > 0x40000) bank |= (data >> 4) & 8;

	/* when they set the AB bank, it appears as though the CD bank is reset */
	if (data & 0x20)
	{
		memory_set_bank(space->machine, "bank1", bank);
		memory_set_bank(space->machine, "bank2", 6);
	}

	/* set both banks */
	else
	{
		memory_set_bank(space->machine, "bank1", bank);
		memory_set_bank(space->machine, "bank2", bank);
	}
}



/*************************************
 *
 *  Special outputs
 *
 *************************************/

WRITE8_HANDLER( balsente_misc_output_w )
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
//      set_led_status(space->machine, offset, data);
	}
}



/*************************************
 *
 *  6850 UART communications
 *
 *************************************/

static void m6850_update_io(running_machine *machine)
{
	balsente_state *state = machine->driver_data<balsente_state>();
	UINT8 new_state;

	/* sound -> main CPU communications */
	if (!(state->m6850_sound_status & 0x02))
	{
		/* set the overrun bit if the data in the destination hasn't been read yet */
		if (state->m6850_status & 0x01)
			state->m6850_status |= 0x20;

		/* copy the sound's output to our input */
		state->m6850_input = state->m6850_sound_output;

		/* set the receive register full bit */
		state->m6850_status |= 0x01;

		/* set the sound's trasmitter register empty bit */
		state->m6850_sound_status |= 0x02;
	}

	/* main -> sound CPU communications */
	if (state->m6850_data_ready)
	{
		/* set the overrun bit if the data in the destination hasn't been read yet */
		if (state->m6850_sound_status & 0x01)
			state->m6850_sound_status |= 0x20;

		/* copy the main CPU's output to our input */
		state->m6850_sound_input = state->m6850_output;

		/* set the receive register full bit */
		state->m6850_sound_status |= 0x01;

		/* set the main CPU's trasmitter register empty bit */
		state->m6850_status |= 0x02;
		state->m6850_data_ready = 0;
	}

	/* check for reset states */
	if ((state->m6850_control & 3) == 3)
	{
		state->m6850_status = 0x02;
		state->m6850_data_ready = 0;
	}
	if ((state->m6850_sound_control & 3) == 3)
		state->m6850_sound_status = 0x02;

	/* check for transmit/receive IRQs on the main CPU */
	new_state = 0;
	if ((state->m6850_control & 0x80) && (state->m6850_status & 0x21)) new_state = 1;
	if ((state->m6850_control & 0x60) == 0x20 && (state->m6850_status & 0x02)) new_state = 1;

	/* apply the change */
	if (new_state && !(state->m6850_status & 0x80))
	{
		cputag_set_input_line(machine, "maincpu", M6809_FIRQ_LINE, ASSERT_LINE);
		state->m6850_status |= 0x80;
	}
	else if (!new_state && (state->m6850_status & 0x80))
	{
		cputag_set_input_line(machine, "maincpu", M6809_FIRQ_LINE, CLEAR_LINE);
		state->m6850_status &= ~0x80;
	}

	/* check for transmit/receive IRQs on the sound CPU */
	new_state = 0;
	if ((state->m6850_sound_control & 0x80) && (state->m6850_sound_status & 0x21)) new_state = 1;
	if ((state->m6850_sound_control & 0x60) == 0x20 && (state->m6850_sound_status & 0x02)) new_state = 1;
	if (!(state->counter_control & 0x20)) new_state = 0;

	/* apply the change */
	if (new_state && !(state->m6850_sound_status & 0x80))
	{
		cputag_set_input_line(machine, "audiocpu", INPUT_LINE_NMI, ASSERT_LINE);
		state->m6850_sound_status |= 0x80;
	}
	else if (!new_state && (state->m6850_sound_status & 0x80))
	{
		cputag_set_input_line(machine, "audiocpu", INPUT_LINE_NMI, CLEAR_LINE);
		state->m6850_sound_status &= ~0x80;
	}
}



/*************************************
 *
 *  6850 UART (main CPU)
 *
 *************************************/

READ8_HANDLER( balsente_m6850_r )
{
	balsente_state *state = space->machine->driver_data<balsente_state>();
	int result;

	/* status register is at offset 0 */
	if (offset == 0)
	{
		result = state->m6850_status;
	}

	/* input register is at offset 1 */
	else
	{
		result = state->m6850_input;

		/* clear the overrun and receive buffer full bits */
		state->m6850_status &= ~0x21;
		m6850_update_io(space->machine);
	}

	return result;
}


static TIMER_CALLBACK( m6850_data_ready_callback )
{
	balsente_state *state = machine->driver_data<balsente_state>();

	/* set the output data byte and indicate that we're ready to go */
	state->m6850_output = param;
	state->m6850_data_ready = 1;
	m6850_update_io(machine);
}


static TIMER_CALLBACK( m6850_w_callback )
{
	balsente_state *state = machine->driver_data<balsente_state>();

	/* indicate that the transmit buffer is no longer empty and update the I/O state */
	state->m6850_status &= ~0x02;
	m6850_update_io(machine);

	/* set a timer for 500usec later to actually transmit the data */
	/* (this is very important for several games, esp Snacks'n Jaxson) */
	timer_set(machine, ATTOTIME_IN_USEC(500), NULL, param, m6850_data_ready_callback);
}


WRITE8_HANDLER( balsente_m6850_w )
{
	balsente_state *state = space->machine->driver_data<balsente_state>();

	/* control register is at offset 0 */
	if (offset == 0)
	{
		state->m6850_control = data;

		/* re-update since interrupt enables could have been modified */
		m6850_update_io(space->machine);
	}

	/* output register is at offset 1; set a timer to synchronize the CPUs */
	else
		timer_call_after_resynch(space->machine, NULL, data, m6850_w_callback);
}



/*************************************
 *
 *  6850 UART (sound CPU)
 *
 *************************************/

READ8_HANDLER( balsente_m6850_sound_r )
{
	balsente_state *state = space->machine->driver_data<balsente_state>();
	int result;

	/* status register is at offset 0 */
	if (offset == 0)
	{
		result = state->m6850_sound_status;
	}

	/* input register is at offset 1 */
	else
	{
		result = state->m6850_sound_input;

		/* clear the overrun and receive buffer full bits */
		state->m6850_sound_status &= ~0x21;
		m6850_update_io(space->machine);
	}

	return result;
}


WRITE8_HANDLER( balsente_m6850_sound_w )
{
	balsente_state *state = space->machine->driver_data<balsente_state>();

	/* control register is at offset 0 */
	if (offset == 0)
		state->m6850_sound_control = data;

	/* output register is at offset 1 */
	else
	{
		state->m6850_sound_output = data;
		state->m6850_sound_status &= ~0x02;
	}

	/* re-update since interrupt enables could have been modified */
	m6850_update_io(space->machine);
}



/*************************************
 *
 *  ADC handlers
 *
 *************************************/

INTERRUPT_GEN( balsente_update_analog_inputs )
{
	balsente_state *state = device->machine->driver_data<balsente_state>();
	int i;
	static const char *const analog[] = { "AN0", "AN1", "AN2", "AN3" };

	/* the analog input system helpfully scales the value read by the percentage of time */
	/* into the current frame we are; unfortunately, this is bad for us, since the analog */
	/* ports are read once a frame, just at varying intervals. To get around this, we */
	/* read all the analog inputs at VBLANK time and just return the cached values. */
	for (i = 0; i < 4; i++)
		state->analog_input_data[i] = input_port_read(device->machine, analog[i]);
}


static TIMER_CALLBACK( adc_finished )
{
	balsente_state *state = machine->driver_data<balsente_state>();
	int which = param;

	/* analog controls are read in two pieces; the lower port returns the sign */
	/* and the upper port returns the absolute value of the magnitude */
	int val = state->analog_input_data[which / 2] << state->adc_shift;

	/* special case for Stompin'/Shrike Avenger */
	if (state->adc_shift == 32)
	{
		state->adc_value = state->analog_input_data[which];
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
		state->adc_value = (val < 0) ? 0xff : 0x00;

	/* return the magnitude */
	else
		state->adc_value = (val < 0) ? -val : val;
}


READ8_HANDLER( balsente_adc_data_r )
{
	balsente_state *state = space->machine->driver_data<balsente_state>();

	/* just return the last value read */
	return state->adc_value;
}


WRITE8_HANDLER( balsente_adc_select_w )
{
	/* set a timer to go off and read the value after 50us */
	/* it's important that we do this for Mini Golf */
logerror("adc_select %d\n", offset & 7);
	timer_set(space->machine, ATTOTIME_IN_USEC(50), NULL, offset & 7, adc_finished);
}



/*************************************
 *
 *  8253-5 timer utilities
 *
 *  NOTE: this is far from complete!
 *
 *************************************/

INLINE void counter_start(balsente_state *state, int which)
{
	/* don't start a timer for channel 0; it is clocked manually */
	if (which != 0)
	{
		/* only start a timer if we're gated and there is none already */
		if (state->counter[which].gate && !state->counter[which].timer_active)
		{
			state->counter[which].timer_active = 1;
			state->counter[which].timer->adjust(attotime_mul(ATTOTIME_IN_HZ(2000000), state->counter[which].count), which);
		}
	}
}


INLINE void counter_stop(balsente_state *state, int which)
{
	/* only stop the timer if it exists */
	if (state->counter[which].timer_active)
		state->counter[which].timer->reset();
	state->counter[which].timer_active = 0;
}


INLINE void counter_update_count(balsente_state *state, int which)
{
	/* only update if the timer is running */
	if (state->counter[which].timer_active)
	{
		/* determine how many 2MHz cycles are remaining */
		int count = attotime_to_double(attotime_mul(state->counter[which].timer->time_left(), 2000000));
		state->counter[which].count = (count < 0) ? 0 : count;
	}
}



/*************************************
 *
 *  8253-5 timer internals
 *
 *  NOTE: this is far from complete!
 *
 *************************************/

static void counter_set_gate(running_machine *machine, int which, int gate)
{
	balsente_state *state = machine->driver_data<balsente_state>();
	int oldgate = state->counter[which].gate;

	/* remember the gate state */
	state->counter[which].gate = gate;

	/* if the counter is being halted, update the count and remove the system timer */
	if (!gate && oldgate)
	{
		counter_update_count(state, which);
		counter_stop(state, which);
	}

	/* if the counter is being started, create the timer */
	else if (gate && !oldgate)
	{
		/* mode 1 waits for the gate to trigger the counter */
		if (state->counter[which].mode == 1)
		{
			counter_set_out(machine, which, 0);

			/* add one to the count; technically, OUT goes low on the next clock pulse */
			/* and then starts counting down; it's important that we don't count the first one */
			state->counter[which].count = state->counter[which].initial + 1;
		}

		/* start the counter */
		counter_start(state, which);
	}
}


static void counter_set_out(running_machine *machine, int which, int out)
{
	balsente_state *state = machine->driver_data<balsente_state>();

	/* OUT on counter 2 is hooked to the /INT line on the Z80 */
	if (which == 2)
		cputag_set_input_line(machine, "audiocpu", 0, out ? ASSERT_LINE : CLEAR_LINE);

	/* OUT on counter 0 is hooked to the GATE line on counter 1 */
	else if (which == 0)
		counter_set_gate(machine, 1, !out);

	/* remember the out state */
	state->counter[which].out = out;
}


TIMER_DEVICE_CALLBACK( balsente_counter_callback )
{
	balsente_state *state = timer.machine->driver_data<balsente_state>();

	/* reset the counter and the count */
	state->counter[param].timer_active = 0;
	state->counter[param].count = 0;

	/* set the state of the OUT line */
	/* mode 0 and 1: when firing, transition OUT to high */
	if (state->counter[param].mode == 0 || state->counter[param].mode == 1)
		counter_set_out(timer.machine, param, 1);

	/* no other modes handled currently */
}



/*************************************
 *
 *  8253-5 timer handlers
 *
 *  NOTE: this is far from complete!
 *
 *************************************/

READ8_HANDLER( balsente_counter_8253_r )
{
	balsente_state *state = space->machine->driver_data<balsente_state>();
	int which;

	switch (offset & 3)
	{
		case 0:
		case 1:
		case 2:
			/* warning: assumes LSB/MSB addressing and no latching! */
			which = offset & 3;

			/* update the count */
			counter_update_count(state, which);

			/* return the LSB */
			if (state->counter[which].readbyte == 0)
			{
				state->counter[which].readbyte = 1;
				return state->counter[which].count & 0xff;
			}

			/* write the MSB and reset the counter */
			else
			{
				state->counter[which].readbyte = 0;
				return (state->counter[which].count >> 8) & 0xff;
			}
			break;
	}
	return 0;
}


WRITE8_HANDLER( balsente_counter_8253_w )
{
	balsente_state *state = space->machine->driver_data<balsente_state>();
	int which;

	switch (offset & 3)
	{
		case 0:
		case 1:
		case 2:
			/* warning: assumes LSB/MSB addressing and no latching! */
			which = offset & 3;

			/* if the counter is in mode 0, a write here will reset the OUT state */
			if (state->counter[which].mode == 0)
				counter_set_out(space->machine, which, 0);

			/* write the LSB */
			if (state->counter[which].writebyte == 0)
			{
				state->counter[which].count = (state->counter[which].count & 0xff00) | (data & 0x00ff);
				state->counter[which].initial = (state->counter[which].initial & 0xff00) | (data & 0x00ff);
				state->counter[which].writebyte = 1;
			}

			/* write the MSB and reset the counter */
			else
			{
				state->counter[which].count = (state->counter[which].count & 0x00ff) | ((data << 8) & 0xff00);
				state->counter[which].initial = (state->counter[which].initial & 0x00ff) | ((data << 8) & 0xff00);
				state->counter[which].writebyte = 0;

				/* treat 0 as $10000 */
				if (state->counter[which].count == 0) state->counter[which].count = state->counter[which].initial = 0x10000;

				/* remove any old timer and set a new one */
				counter_stop(state, which);

				/* note that in mode 1, we have to wait for a rising edge of a gate */
				if (state->counter[which].mode == 0)
					counter_start(state, which);

				/* if the counter is in mode 1, a write here will set the OUT state */
				if (state->counter[which].mode == 1)
					counter_set_out(space->machine, which, 1);
			}
			break;

		case 3:
			/* determine which counter */
			which = data >> 6;
			if (which == 3) break;

			/* if the counter was in mode 0, a write here will reset the OUT state */
			if (((state->counter[which].mode >> 1) & 7) == 0)
				counter_set_out(space->machine, which, 0);

			/* set the mode */
			state->counter[which].mode = (data >> 1) & 7;

			/* if the counter is in mode 0, a write here will reset the OUT state */
			if (state->counter[which].mode == 0)
				counter_set_out(space->machine, which, 0);
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
	balsente_state *state = timer.machine->driver_data<balsente_state>();

	/* the flip/flop output is inverted, so if we went high to low, that's a clock */
	if (state->counter_0_ff && !newstate)
	{
		/* only count if gated and non-zero */
		if (state->counter[0].count > 0 && state->counter[0].gate)
		{
			state->counter[0].count--;
			if (state->counter[0].count == 0)
				balsente_counter_callback(timer, NULL, 0);
		}
	}

	/* remember the new state */
	state->counter_0_ff = newstate;
}


TIMER_DEVICE_CALLBACK( balsente_clock_counter_0_ff )
{
	balsente_state *state = timer.machine->driver_data<balsente_state>();

	/* clock the D value through the flip-flop */
	set_counter_0_ff(timer, (state->counter_control >> 3) & 1);
}


static void update_counter_0_timer(balsente_state *state)
{
	double maxfreq = 0.0;
	int i;

	/* if there's already a timer, remove it */
	if (state->counter_0_timer_active)
		state->counter_0_timer->reset();
	state->counter_0_timer_active = 0;

	/* find the counter with the maximum frequency */
	/* this is used to calibrate the timers at startup */
	for (i = 0; i < 6; i++)
		if (cem3394_get_parameter(state->cem_device[i], CEM3394_FINAL_GAIN) < 10.0)
		{
			double tempfreq;

			/* if the filter resonance is high, then they're calibrating the filter frequency */
			if (cem3394_get_parameter(state->cem_device[i], CEM3394_FILTER_RESONANCE) > 0.9)
				tempfreq = cem3394_get_parameter(state->cem_device[i], CEM3394_FILTER_FREQENCY);

			/* otherwise, they're calibrating the VCO frequency */
			else
				tempfreq = cem3394_get_parameter(state->cem_device[i], CEM3394_VCO_FREQUENCY);

			if (tempfreq > maxfreq) maxfreq = tempfreq;
		}

	/* reprime the timer */
	if (maxfreq > 0.0)
	{
		state->counter_0_timer_active = 1;
		state->counter_0_timer->adjust(ATTOTIME_IN_HZ(maxfreq), 0, ATTOTIME_IN_HZ(maxfreq));
	}
}



/*************************************
 *
 *  Sound CPU counter handlers
 *
 *************************************/

READ8_HANDLER( balsente_counter_state_r )
{
	balsente_state *state = space->machine->driver_data<balsente_state>();

	/* bit D0 is the inverse of the flip-flop state */
	int result = !state->counter_0_ff;

	/* bit D1 is the OUT value from counter 0 */
	if (state->counter[0].out) result |= 0x02;

	return result;
}


WRITE8_HANDLER( balsente_counter_control_w )
{
	balsente_state *state = space->machine->driver_data<balsente_state>();
	UINT8 diff_counter_control = state->counter_control ^ data;

	/* set the new global value */
	state->counter_control = data;

	/* bit D0 enables/disables audio */
	if (diff_counter_control & 0x01)
	{
		int ch;
		for (ch = 0; ch < 6; ch++)
			sound_set_output_gain(state->cem_device[ch], 0, (data & 0x01) ? 1.0 : 0);
	}

	/* bit D1 is hooked to counter 0's gate */
	/* if we gate on, start a pulsing timer to clock it */
	if (!state->counter[0].gate && (data & 0x02) && !state->counter_0_timer_active)
	{
		update_counter_0_timer(state);
	}

	/* if we gate off, remove the timer */
	else if (state->counter[0].gate && !(data & 0x02) && state->counter_0_timer_active)
	{
		state->counter_0_timer->reset();
		state->counter_0_timer_active = 0;
	}

	/* set the actual gate afterwards, since we need to know the old value above */
	counter_set_gate(space->machine, 0, (data >> 1) & 1);

	/* bits D2 and D4 control the clear/reset flags on the flip-flop that feeds counter 0 */
	if (!(data & 0x04)) set_counter_0_ff(*state->counter_0_timer, 1);
	if (!(data & 0x10)) set_counter_0_ff(*state->counter_0_timer, 0);

	/* bit 5 clears the NMI interrupt; recompute the I/O state now */
	m6850_update_io(space->machine);
}



/*************************************
 *
 *  CEM3394 Interfaces
 *
 *************************************/

WRITE8_HANDLER( balsente_chip_select_w )
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

	balsente_state *state = space->machine->driver_data<balsente_state>();
	double voltage = (double)state->dac_value * (8.0 / 4096.0) - 4.0;
	int diffchip = data ^ state->chip_select, i;
	int reg = register_map[state->dac_register];

	/* remember the new select value */
	state->chip_select = data;

	/* check all six chip enables */
	for (i = 0; i < 6; i++)
		if ((diffchip & (1 << i)) && (data & (1 << i)))
		{
#if LOG_CEM_WRITES
			double temp = 0;

			/* remember the previous value */
			temp =
#endif
				cem3394_get_parameter(state->cem_device[i], reg);

			/* set the voltage */
			cem3394_set_voltage(state->cem_device[i], reg, voltage);

			/* only log changes */
#if LOG_CEM_WRITES
			if (temp != cem3394_get_parameter(state->cem_device[i], reg))
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
				logerror("s%04X:   CEM#%d:%s=%f\n", cpu_get_previouspc(space->cpu), i, names[state->dac_register], voltage);
			}
#endif
		}

	/* if a timer for counter 0 is running, recompute */
	if (state->counter_0_timer_active)
		update_counter_0_timer(state);
}



WRITE8_HANDLER( balsente_dac_data_w )
{
	balsente_state *state = space->machine->driver_data<balsente_state>();

	/* LSB or MSB? */
	if (offset & 1)
		state->dac_value = (state->dac_value & 0xfc0) | ((data >> 2) & 0x03f);
	else
		state->dac_value = (state->dac_value & 0x03f) | ((data << 6) & 0xfc0);

	/* if there are open channels, force the values in */
	if ((state->chip_select & 0x3f) != 0x3f)
	{
		UINT8 temp = state->chip_select;
		balsente_chip_select_w(space, 0, 0x3f);
		balsente_chip_select_w(space, 0, temp);
	}
}


WRITE8_HANDLER( balsente_register_addr_w )
{
	balsente_state *state = space->machine->driver_data<balsente_state>();
	state->dac_register = data & 7;
}



/*************************************
 *
 *  Game-specific handlers
 *
 *************************************/

CUSTOM_INPUT( nstocker_bits_r )
{
	balsente_state *state = field->port->machine->driver_data<balsente_state>();
	return state->nstocker_bits;
}


WRITE8_HANDLER( spiker_expand_w )
{
	balsente_state *state = space->machine->driver_data<balsente_state>();

	/* offset 0 is the bit pattern */
	if (offset == 0)
		state->spiker_expand_bits = data;

	/* offset 1 is the background color (cleared on each read) */
	else if (offset == 1)
		state->spiker_expand_bgcolor = data;

	/* offset 2 is the color */
	else if (offset == 2)
		state->spiker_expand_color = data;
}


READ8_HANDLER( spiker_expand_r )
{
	balsente_state *state = space->machine->driver_data<balsente_state>();
	UINT8 left, right;

	/* first rotate each nibble */
	state->spiker_expand_bits = ((state->spiker_expand_bits << 1) & 0xee) | ((state->spiker_expand_bits >> 3) & 0x11);

	/* compute left and right pixels */
	left  = (state->spiker_expand_bits & 0x10) ? state->spiker_expand_color : state->spiker_expand_bgcolor;
	right = (state->spiker_expand_bits & 0x01) ? state->spiker_expand_color : state->spiker_expand_bgcolor;

	/* reset the background color */
	state->spiker_expand_bgcolor = 0;

	/* return the combined result */
	return (left & 0xf0) | (right & 0x0f);
}


static void update_grudge_steering(running_machine *machine)
{
	balsente_state *state = machine->driver_data<balsente_state>();
	UINT8 wheel[3];
	INT8 diff[3];

	/* read the current steering values */
	wheel[0] = input_port_read(machine, "AN0");
	wheel[1] = input_port_read(machine, "AN1");
	wheel[2] = input_port_read(machine, "AN2");

	/* diff the values */
	diff[0] = wheel[0] - state->grudge_last_steering[0];
	diff[1] = wheel[1] - state->grudge_last_steering[1];
	diff[2] = wheel[2] - state->grudge_last_steering[2];

	/* update the last values */
	state->grudge_last_steering[0] += diff[0];
	state->grudge_last_steering[1] += diff[1];
	state->grudge_last_steering[2] += diff[2];

	/* compute the result */
	state->grudge_steering_result = 0xff;
	if (diff[0])
	{
		state->grudge_steering_result ^= 0x01;
		if (diff[0] > 0) state->grudge_steering_result ^= 0x02;
	}
	if (diff[1])
	{
		state->grudge_steering_result ^= 0x04;
		if (diff[1] > 0) state->grudge_steering_result ^= 0x08;
	}
	if (diff[2])
	{
		state->grudge_steering_result ^= 0x10;
		if (diff[2] > 0) state->grudge_steering_result ^= 0x20;
	}
	logerror("Recomputed steering\n");
}


READ8_HANDLER( grudge_steering_r )
{
	balsente_state *state = space->machine->driver_data<balsente_state>();
	logerror("%04X:grudge_steering_r(@%d)\n", cpu_get_pc(space->cpu), space->machine->primary_screen->vpos());
	state->grudge_steering_result |= 0x80;
	return state->grudge_steering_result;
}



/*************************************
 *
 *  Shrike Avenger CPU memory handlers
 *
 *************************************/

READ8_HANDLER( shrike_shared_6809_r )
{
	balsente_state *state = space->machine->driver_data<balsente_state>();
	UINT16 mem_mask = offset & 1 ? 0xff00 : 0x00ff;

	switch( offset )
	{
		case 6: // return OK for 68k status register until motors hooked up
			return 0;
		default:
			return ( state->shrike_shared[offset >> 1] & ~mem_mask ) >> ( mem_mask & 8 );
	}
}


WRITE8_HANDLER( shrike_shared_6809_w )
{
	balsente_state *state = space->machine->driver_data<balsente_state>();
	UINT16 mem_mask = offset & 1 ? 0xff00 : 0x00ff;
	state->shrike_shared[offset >> 1] = ( state->shrike_shared[offset >> 1] & mem_mask ) | ( data << ( mem_mask & 0x8 ) );
}

// uses movep, so writes even 8 bit addresses to odd 16 bit addresses, reads as 16 bit from odd addresses
// i.e. write 0xdeadbeef to 10000, read 0xde from 10001, 0xad from 10003, 0xbe from 10005...
WRITE16_HANDLER( shrike_io_68k_w )
{
	balsente_state *state = space->machine->driver_data<balsente_state>();
	COMBINE_DATA( &state->shrike_io[offset] );
}

READ16_HANDLER( shrike_io_68k_r )
{
	balsente_state *state = space->machine->driver_data<balsente_state>();
	return ( state->shrike_io[offset] & mem_mask ) >> ( 8 & ~mem_mask );
}

