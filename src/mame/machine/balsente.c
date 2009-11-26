/***************************************************************************

    Bally/Sente SAC-1 system

    driver by Aaron Giles

***************************************************************************/

#include "driver.h"
#include "cpu/m6809/m6809.h"
#include "balsente.h"
#include "sound/cem3394.h"


#define LOG_CEM_WRITES		0


/* local prototypes */
static void poly17_init(running_machine *machine);
static void counter_set_out(running_machine *machine, int which, int gate);
static TIMER_CALLBACK( counter_callback );
static TIMER_CALLBACK( clock_counter_0_ff );
static void update_grudge_steering(running_machine *machine);

/* global data */
UINT8 balsente_shooter;
static UINT8 balsente_shooter_x;
static UINT8 balsente_shooter_y;
UINT8 balsente_adc_shift;
UINT16 *shrike_shared;
UINT16 *shrike_io;


/* 8253 counter state */
struct counter_state
{
	emu_timer *timer;
	UINT8 timer_active;
	INT32 initial;
	INT32 count;
	UINT8 gate;
	UINT8 out;
	UINT8 mode;
	UINT8 readbyte;
	UINT8 writebyte;
};

static struct counter_state counter[3];

static emu_timer *scanline_timer;

/* manually clocked counter 0 states */
static UINT8 counter_control;
static UINT8 counter_0_ff;
static emu_timer *counter_0_timer;
static UINT8 counter_0_timer_active;

/* random number generator states */
static UINT8 *poly17 = NULL;
static UINT8 *rand17 = NULL;

/* ADC I/O states */
static INT8 analog_input_data[4];
static UINT8 adc_value;

/* CEM3394 DAC control states */
static UINT16 dac_value;
static UINT8 dac_register;
static UINT8 chip_select;

/* main CPU 6850 states */
static UINT8 m6850_status;
static UINT8 m6850_control;
static UINT8 m6850_input;
static UINT8 m6850_output;
static UINT8 m6850_data_ready;

/* sound CPU 6850 states */
static UINT8 m6850_sound_status;
static UINT8 m6850_sound_control;
static UINT8 m6850_sound_input;
static UINT8 m6850_sound_output;

/* noise generator states */
static UINT32 noise_position[6];
static const device_config *cem_device[6];

/* game-specific states */
static UINT8 nstocker_bits;
static UINT8 spiker_expand_color;
static UINT8 spiker_expand_bgcolor;
static UINT8 spiker_expand_bits;
static UINT8 grudge_steering_result;
static UINT8 grudge_last_steering[3];



/*************************************
 *
 *  Interrupt handling
 *
 *************************************/

static TIMER_CALLBACK( irq_off )
{
	cputag_set_input_line(machine, "maincpu", M6809_IRQ_LINE, CLEAR_LINE);
}


static TIMER_CALLBACK( interrupt_timer )
{
	/* next interrupt after scanline 256 is scanline 64 */
	if (param == 256)
		timer_adjust_oneshot(scanline_timer, video_screen_get_time_until_pos(machine->primary_screen, 64, 0), 64);
	else
		timer_adjust_oneshot(scanline_timer, video_screen_get_time_until_pos(machine->primary_screen, param + 64, 0), param + 64);

	/* IRQ starts on scanline 0, 64, 128, etc. */
	cputag_set_input_line(machine, "maincpu", M6809_IRQ_LINE, ASSERT_LINE);

	/* it will turn off on the next HBLANK */
	timer_set(machine, video_screen_get_time_until_pos(machine->primary_screen, param, BALSENTE_HBSTART), NULL, 0, irq_off);

	/* if this is Grudge Match, update the steering */
	if (grudge_steering_result & 0x80)
		update_grudge_steering(machine);

	/* if we're a shooter, we do a little more work */
	if (balsente_shooter)
	{
		UINT8 tempx, tempy;

		/* we latch the beam values on the first interrupt after VBLANK */
		if (param == 64 && balsente_shooter)
		{
			balsente_shooter_x = input_port_read(machine, "FAKEX");
			balsente_shooter_y = input_port_read(machine, "FAKEY");
		}

		/* which bits get returned depends on which scanline we're at */
		tempx = balsente_shooter_x << ((param - 64) / 64);
		tempy = balsente_shooter_y << ((param - 64) / 64);
		nstocker_bits = ((tempx >> 4) & 0x08) | ((tempx >> 1) & 0x04) |
						((tempy >> 6) & 0x02) | ((tempy >> 3) & 0x01);
	}
}


MACHINE_RESET( balsente )
{
	const address_space *space = cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM);
	int numbanks, i;

	/* create the polynomial tables */
	poly17_init(machine);

	/* reset counters; counter 2's gate is tied high */
	memset(counter, 0, sizeof(counter));
	counter[1].timer = timer_alloc(machine, counter_callback, NULL);
	counter[2].timer = timer_alloc(machine, counter_callback, NULL);
	counter[2].gate = 1;

	/* reset the manual counter 0 clock */
	counter_control = 0x00;
	counter_0_ff = 0;
	counter_0_timer = timer_alloc(machine, clock_counter_0_ff, NULL);
	counter_0_timer_active = 0;

	/* reset the ADC states */
	adc_value = 0;

	/* reset the CEM3394 I/O states */
	dac_value = 0;
	dac_register = 0;
	chip_select = 0x3f;

	/* reset game-specific states */
	grudge_steering_result = 0;

	/* reset the 6850 chips */
	balsente_m6850_w(space, 0, 3);
	balsente_m6850_sound_w(space, 0, 3);

	/* reset the noise generator */
	memset(noise_position, 0, sizeof(noise_position));

	/* find the CEM devices */
	for (i = 0; i < ARRAY_LENGTH(cem_device); i++)
	{
		char name[10];
		sprintf(name, "cem%d", i+1);
		cem_device[i] = devtag_get_device(machine, name);
		assert(cem_device[i] != NULL);
	}

	/* point the banks to bank 0 */
	numbanks = (memory_region_length(machine, "maincpu") > 0x40000) ? 16 : 8;
	memory_configure_bank(machine, 1, 0, numbanks, &memory_region(machine, "maincpu")[0x10000], 0x6000);
	memory_configure_bank(machine, 2, 0, numbanks, &memory_region(machine, "maincpu")[0x12000], 0x6000);
	memory_set_bank(space->machine, 1, 0);
	memory_set_bank(space->machine, 2, 0);
	device_reset(cputag_get_cpu(machine, "maincpu"));

	/* start a timer to generate interrupts */
	scanline_timer = timer_alloc(machine, interrupt_timer, NULL);
	timer_adjust_oneshot(scanline_timer, video_screen_get_time_until_pos(machine->primary_screen, 0, 0), 0);

	/* register for saving */
	for (i = 0; i < 3; i++)
	{
		state_save_register_item(machine, "8253counter", NULL, i, counter[i].timer_active);
		state_save_register_item(machine, "8253counter", NULL, i, counter[i].initial);
		state_save_register_item(machine, "8253counter", NULL, i, counter[i].count);
		state_save_register_item(machine, "8253counter", NULL, i, counter[i].gate);
		state_save_register_item(machine, "8253counter", NULL, i, counter[i].out);
		state_save_register_item(machine, "8253counter", NULL, i, counter[i].mode);
		state_save_register_item(machine, "8253counter", NULL, i, counter[i].readbyte);
		state_save_register_item(machine, "8253counter", NULL, i, counter[i].writebyte);
	}

	state_save_register_global(machine, counter_control);
	state_save_register_global(machine, counter_0_ff);
	state_save_register_global(machine, counter_0_timer_active);

	state_save_register_global_array(machine, analog_input_data);
	state_save_register_global(machine, adc_value);

	state_save_register_global(machine, dac_value);
	state_save_register_global(machine, dac_register);
	state_save_register_global(machine, chip_select);

	state_save_register_global(machine, m6850_status);
	state_save_register_global(machine, m6850_control);
	state_save_register_global(machine, m6850_input);
	state_save_register_global(machine, m6850_output);
	state_save_register_global(machine, m6850_data_ready);

	state_save_register_global(machine, m6850_sound_status);
	state_save_register_global(machine, m6850_sound_control);
	state_save_register_global(machine, m6850_sound_input);
	state_save_register_global(machine, m6850_sound_output);

	state_save_register_global_array(machine, noise_position);

	state_save_register_global(machine, nstocker_bits);
	state_save_register_global(machine, spiker_expand_color);
	state_save_register_global(machine, spiker_expand_bgcolor);
	state_save_register_global(machine, spiker_expand_bits);
	state_save_register_global(machine, grudge_steering_result);
	state_save_register_global_array(machine, grudge_last_steering);
}



/*************************************
 *
 *  MM5837 noise generator
 *
 *  NOTE: this is stolen straight from
 *          POKEY.c
 *
 *************************************/

#define POLY17_BITS 17
#define POLY17_SIZE ((1 << POLY17_BITS) - 1)
#define POLY17_SHL	7
#define POLY17_SHR	10
#define POLY17_ADD	0x18000

static void poly17_init(running_machine *machine)
{
	UINT32 i, x = 0;
	UINT8 *p, *r;

	/* allocate memory */
	p = poly17 = auto_alloc_array(machine, UINT8, POLY17_SIZE + 1);
	r = rand17 = auto_alloc_array(machine, UINT8, POLY17_SIZE + 1);

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


void balsente_noise_gen(const device_config *device, int count, short *buffer)
{
	int chip;
	UINT32 step, noise_counter;

	/* find the chip we are referring to */
	for (chip = 0; chip < ARRAY_LENGTH(cem_device); chip++)
	{
		if (device == cem_device[chip])
			break;
		else if (cem_device[chip] == NULL)
		{
			cem_device[chip] = device;
			break;
		}
	}
	assert(chip < ARRAY_LENGTH(cem_device));

	/* noise generator runs at 100kHz */
	step = (100000 << 14) / CEM3394_SAMPLE_RATE;
	noise_counter = noise_position[chip];

	while (count--)
	{
		*buffer++ = poly17[(noise_counter >> 14) & POLY17_SIZE] << 12;
		noise_counter += step;
	}

	/* remember the noise position */
	noise_position[chip] = noise_counter;
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
	UINT32 cc;

	/* CPU runs at 1.25MHz, noise source at 100kHz --> multiply by 12.5 */
	cc = cpu_get_total_cycles(space->cpu);

	/* 12.5 = 8 + 4 + 0.5 */
	cc = (cc << 3) + (cc << 2) + (cc >> 1);
	return rand17[cc & POLY17_SIZE];
}



/*************************************
 *
 *  ROM banking
 *
 *************************************/

WRITE8_HANDLER( balsente_rombank_select_w )
{
	/* the bank number comes from bits 4-6 */
	memory_set_bank(space->machine, 1, (data >> 4) & 7);
	memory_set_bank(space->machine, 2, (data >> 4) & 7);
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
		memory_set_bank(space->machine, 1, bank);
		memory_set_bank(space->machine, 2, 6);
	}

	/* set both banks */
	else
	{
		memory_set_bank(space->machine, 1, bank);
		memory_set_bank(space->machine, 2, bank);
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
	UINT8 new_state;

	/* sound -> main CPU communications */
	if (!(m6850_sound_status & 0x02))
	{
		/* set the overrun bit if the data in the destination hasn't been read yet */
		if (m6850_status & 0x01)
			m6850_status |= 0x20;

		/* copy the sound's output to our input */
		m6850_input = m6850_sound_output;

		/* set the receive register full bit */
		m6850_status |= 0x01;

		/* set the sound's trasmitter register empty bit */
		m6850_sound_status |= 0x02;
	}

	/* main -> sound CPU communications */
	if (m6850_data_ready)
	{
		/* set the overrun bit if the data in the destination hasn't been read yet */
		if (m6850_sound_status & 0x01)
			m6850_sound_status |= 0x20;

		/* copy the main CPU's output to our input */
		m6850_sound_input = m6850_output;

		/* set the receive register full bit */
		m6850_sound_status |= 0x01;

		/* set the main CPU's trasmitter register empty bit */
		m6850_status |= 0x02;
		m6850_data_ready = 0;
	}

	/* check for reset states */
	if ((m6850_control & 3) == 3)
	{
		m6850_status = 0x02;
		m6850_data_ready = 0;
	}
	if ((m6850_sound_control & 3) == 3)
		m6850_sound_status = 0x02;

	/* check for transmit/receive IRQs on the main CPU */
	new_state = 0;
	if ((m6850_control & 0x80) && (m6850_status & 0x21)) new_state = 1;
	if ((m6850_control & 0x60) == 0x20 && (m6850_status & 0x02)) new_state = 1;

	/* apply the change */
	if (new_state && !(m6850_status & 0x80))
	{
		cputag_set_input_line(machine, "maincpu", M6809_FIRQ_LINE, ASSERT_LINE);
		m6850_status |= 0x80;
	}
	else if (!new_state && (m6850_status & 0x80))
	{
		cputag_set_input_line(machine, "maincpu", M6809_FIRQ_LINE, CLEAR_LINE);
		m6850_status &= ~0x80;
	}

	/* check for transmit/receive IRQs on the sound CPU */
	new_state = 0;
	if ((m6850_sound_control & 0x80) && (m6850_sound_status & 0x21)) new_state = 1;
	if ((m6850_sound_control & 0x60) == 0x20 && (m6850_sound_status & 0x02)) new_state = 1;
	if (!(counter_control & 0x20)) new_state = 0;

	/* apply the change */
	if (new_state && !(m6850_sound_status & 0x80))
	{
		cputag_set_input_line(machine, "audiocpu", INPUT_LINE_NMI, ASSERT_LINE);
		m6850_sound_status |= 0x80;
	}
	else if (!new_state && (m6850_sound_status & 0x80))
	{
		cputag_set_input_line(machine, "audiocpu", INPUT_LINE_NMI, CLEAR_LINE);
		m6850_sound_status &= ~0x80;
	}
}



/*************************************
 *
 *  6850 UART (main CPU)
 *
 *************************************/

READ8_HANDLER( balsente_m6850_r )
{
	int result;

	/* status register is at offset 0 */
	if (offset == 0)
	{
		result = m6850_status;
	}

	/* input register is at offset 1 */
	else
	{
		result = m6850_input;

		/* clear the overrun and receive buffer full bits */
		m6850_status &= ~0x21;
		m6850_update_io(space->machine);
	}

	return result;
}


static TIMER_CALLBACK( m6850_data_ready_callback )
{
	/* set the output data byte and indicate that we're ready to go */
	m6850_output = param;
	m6850_data_ready = 1;
	m6850_update_io(machine);
}


static TIMER_CALLBACK( m6850_w_callback )
{
	/* indicate that the transmit buffer is no longer empty and update the I/O state */
	m6850_status &= ~0x02;
	m6850_update_io(machine);

	/* set a timer for 500usec later to actually transmit the data */
	/* (this is very important for several games, esp Snacks'n Jaxson) */
	timer_set(machine, ATTOTIME_IN_USEC(500), NULL, param, m6850_data_ready_callback);
}


WRITE8_HANDLER( balsente_m6850_w )
{
	/* control register is at offset 0 */
	if (offset == 0)
	{
		m6850_control = data;

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
	int result;

	/* status register is at offset 0 */
	if (offset == 0)
	{
		result = m6850_sound_status;
	}

	/* input register is at offset 1 */
	else
	{
		result = m6850_sound_input;

		/* clear the overrun and receive buffer full bits */
		m6850_sound_status &= ~0x21;
		m6850_update_io(space->machine);
	}

	return result;
}


WRITE8_HANDLER( balsente_m6850_sound_w )
{
	/* control register is at offset 0 */
	if (offset == 0)
		m6850_sound_control = data;

	/* output register is at offset 1 */
	else
	{
		m6850_sound_output = data;
		m6850_sound_status &= ~0x02;
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
	int i;
	static const char *const analog[] = { "AN0", "AN1", "AN2", "AN3" };

	/* the analog input system helpfully scales the value read by the percentage of time */
	/* into the current frame we are; unfortunately, this is bad for us, since the analog */
	/* ports are read once a frame, just at varying intervals. To get around this, we */
	/* read all the analog inputs at VBLANK time and just return the cached values. */
	for (i = 0; i < 4; i++)
		analog_input_data[i] = input_port_read(device->machine, analog[i]);
}


static TIMER_CALLBACK( adc_finished )
{
	int which = param;

	/* analog controls are read in two pieces; the lower port returns the sign */
	/* and the upper port returns the absolute value of the magnitude */
	int val = analog_input_data[which / 2] << balsente_adc_shift;

	/* special case for Stompin'/Shrike Avenger */
	if (balsente_adc_shift == 32)
	{
		adc_value = analog_input_data[which];
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
		adc_value = (val < 0) ? 0xff : 0x00;

	/* return the magnitude */
	else
		adc_value = (val < 0) ? -val : val;
}


READ8_HANDLER( balsente_adc_data_r )
{
	/* just return the last value read */
	return adc_value;
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

INLINE void counter_start(int which)
{
	/* don't start a timer for channel 0; it is clocked manually */
	if (which != 0)
	{
		/* only start a timer if we're gated and there is none already */
		if (counter[which].gate && !counter[which].timer_active)
		{
			counter[which].timer_active = 1;
			timer_adjust_oneshot(counter[which].timer, attotime_mul(ATTOTIME_IN_HZ(2000000), counter[which].count), which);
		}
	}
}


INLINE void counter_stop(int which)
{
	/* only stop the timer if it exists */
	if (counter[which].timer_active)
		timer_adjust_oneshot(counter[which].timer, attotime_never, 0);
	counter[which].timer_active = 0;
}


INLINE void counter_update_count(int which)
{
	/* only update if the timer is running */
	if (counter[which].timer_active)
	{
		/* determine how many 2MHz cycles are remaining */
		int count = attotime_to_double(attotime_mul(timer_timeleft(counter[which].timer), 2000000));
		counter[which].count = (count < 0) ? 0 : count;
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
	int oldgate = counter[which].gate;

	/* remember the gate state */
	counter[which].gate = gate;

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
		if (counter[which].mode == 1)
		{
			counter_set_out(machine, which, 0);

			/* add one to the count; technically, OUT goes low on the next clock pulse */
			/* and then starts counting down; it's important that we don't count the first one */
			counter[which].count = counter[which].initial + 1;
		}

		/* start the counter */
		counter_start(which);
	}
}


static void counter_set_out(running_machine *machine, int which, int out)
{
	/* OUT on counter 2 is hooked to the /INT line on the Z80 */
	if (which == 2)
		cputag_set_input_line(machine, "audiocpu", 0, out ? ASSERT_LINE : CLEAR_LINE);

	/* OUT on counter 0 is hooked to the GATE line on counter 1 */
	else if (which == 0)
		counter_set_gate(machine, 1, !out);

	/* remember the out state */
	counter[which].out = out;
}


static TIMER_CALLBACK( counter_callback )
{
	/* reset the counter and the count */
	counter[param].timer_active = 0;
	counter[param].count = 0;

	/* set the state of the OUT line */
	/* mode 0 and 1: when firing, transition OUT to high */
	if (counter[param].mode == 0 || counter[param].mode == 1)
		counter_set_out(machine, param, 1);

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
			if (counter[which].readbyte == 0)
			{
				counter[which].readbyte = 1;
				return counter[which].count & 0xff;
			}

			/* write the MSB and reset the counter */
			else
			{
				counter[which].readbyte = 0;
				return (counter[which].count >> 8) & 0xff;
			}
			break;
	}
	return 0;
}


WRITE8_HANDLER( balsente_counter_8253_w )
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
			if (counter[which].mode == 0)
				counter_set_out(space->machine, which, 0);

			/* write the LSB */
			if (counter[which].writebyte == 0)
			{
				counter[which].count = (counter[which].count & 0xff00) | (data & 0x00ff);
				counter[which].initial = (counter[which].initial & 0xff00) | (data & 0x00ff);
				counter[which].writebyte = 1;
			}

			/* write the MSB and reset the counter */
			else
			{
				counter[which].count = (counter[which].count & 0x00ff) | ((data << 8) & 0xff00);
				counter[which].initial = (counter[which].initial & 0x00ff) | ((data << 8) & 0xff00);
				counter[which].writebyte = 0;

				/* treat 0 as $10000 */
				if (counter[which].count == 0) counter[which].count = counter[which].initial = 0x10000;

				/* remove any old timer and set a new one */
				counter_stop(which);

				/* note that in mode 1, we have to wait for a rising edge of a gate */
				if (counter[which].mode == 0)
					counter_start(which);

				/* if the counter is in mode 1, a write here will set the OUT state */
				if (counter[which].mode == 1)
					counter_set_out(space->machine, which, 1);
			}
			break;

		case 3:
			/* determine which counter */
			which = data >> 6;
			if (which == 3) break;

			/* if the counter was in mode 0, a write here will reset the OUT state */
			if (((counter[which].mode >> 1) & 7) == 0)
				counter_set_out(space->machine, which, 0);

			/* set the mode */
			counter[which].mode = (data >> 1) & 7;

			/* if the counter is in mode 0, a write here will reset the OUT state */
			if (counter[which].mode == 0)
				counter_set_out(space->machine, which, 0);
			break;
	}
}



/*************************************
 *
 *  Sound CPU counter 0 emulation
 *
 *************************************/

static void set_counter_0_ff(running_machine *machine, int newstate)
{
	/* the flip/flop output is inverted, so if we went high to low, that's a clock */
	if (counter_0_ff && !newstate)
	{
		/* only count if gated and non-zero */
		if (counter[0].count > 0 && counter[0].gate)
		{
			counter[0].count--;
			if (counter[0].count == 0)
				counter_callback(machine, NULL, 0);
		}
	}

	/* remember the new state */
	counter_0_ff = newstate;
}


static TIMER_CALLBACK( clock_counter_0_ff )
{
	/* clock the D value through the flip-flop */
	set_counter_0_ff(machine, (counter_control >> 3) & 1);
}


static void update_counter_0_timer(void)
{
	double maxfreq = 0.0;
	int i;

	/* if there's already a timer, remove it */
	if (counter_0_timer_active)
		timer_adjust_oneshot(counter_0_timer, attotime_never, 0);
	counter_0_timer_active = 0;

	/* find the counter with the maximum frequency */
	/* this is used to calibrate the timers at startup */
	for (i = 0; i < 6; i++)
		if (cem3394_get_parameter(cem_device[i], CEM3394_FINAL_GAIN) < 10.0)
		{
			double tempfreq;

			/* if the filter resonance is high, then they're calibrating the filter frequency */
			if (cem3394_get_parameter(cem_device[i], CEM3394_FILTER_RESONANCE) > 0.9)
				tempfreq = cem3394_get_parameter(cem_device[i], CEM3394_FILTER_FREQENCY);

			/* otherwise, they're calibrating the VCO frequency */
			else
				tempfreq = cem3394_get_parameter(cem_device[i], CEM3394_VCO_FREQUENCY);

			if (tempfreq > maxfreq) maxfreq = tempfreq;
		}

	/* reprime the timer */
	if (maxfreq > 0.0)
	{
		counter_0_timer_active = 1;
		timer_adjust_periodic(counter_0_timer, ATTOTIME_IN_HZ(maxfreq), 0, ATTOTIME_IN_HZ(maxfreq));
	}
}



/*************************************
 *
 *  Sound CPU counter handlers
 *
 *************************************/

READ8_HANDLER( balsente_counter_state_r )
{
	/* bit D0 is the inverse of the flip-flop state */
	int result = !counter_0_ff;

	/* bit D1 is the OUT value from counter 0 */
	if (counter[0].out) result |= 0x02;

	return result;
}


WRITE8_HANDLER( balsente_counter_control_w )
{
	UINT8 diff_counter_control = counter_control ^ data;

	/* set the new global value */
	counter_control = data;

	/* bit D0 enables/disables audio */
	if (diff_counter_control & 0x01)
	{
		int ch;
		for (ch = 0; ch < 6; ch++)
			sound_set_output_gain(cem_device[ch], 0, (data & 0x01) ? 1.0 : 0);
	}

	/* bit D1 is hooked to counter 0's gate */
	/* if we gate on, start a pulsing timer to clock it */
	if (!counter[0].gate && (data & 0x02) && !counter_0_timer_active)
	{
		update_counter_0_timer();
	}

	/* if we gate off, remove the timer */
	else if (counter[0].gate && !(data & 0x02) && counter_0_timer_active)
	{
		timer_adjust_oneshot(counter_0_timer, attotime_never, 0);
		counter_0_timer_active = 0;
	}

	/* set the actual gate afterwards, since we need to know the old value above */
	counter_set_gate(space->machine, 0, (data >> 1) & 1);

	/* bits D2 and D4 control the clear/reset flags on the flip-flop that feeds counter 0 */
	if (!(data & 0x04)) set_counter_0_ff(space->machine, 1);
	if (!(data & 0x10)) set_counter_0_ff(space->machine, 0);

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

	double voltage = (double)dac_value * (8.0 / 4096.0) - 4.0;
	int diffchip = data ^ chip_select, i;
	int reg = register_map[dac_register];

	/* remember the new select value */
	chip_select = data;

	/* check all six chip enables */
	for (i = 0; i < 6; i++)
		if ((diffchip & (1 << i)) && (data & (1 << i)))
		{
#if LOG_CEM_WRITES
			double temp = 0;

			/* remember the previous value */
			temp =
#endif
				cem3394_get_parameter(cem_device[i], reg);

			/* set the voltage */
			cem3394_set_voltage(cem_device[i], reg, voltage);

			/* only log changes */
#if LOG_CEM_WRITES
			if (temp != cem3394_get_parameter(cem_device[i], reg))
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
				logerror("s%04X:   CEM#%d:%s=%f\n", cpu_get_previouspc(space->cpu), i, names[dac_register], voltage);
			}
#endif
		}

	/* if a timer for counter 0 is running, recompute */
	if (counter_0_timer_active)
		update_counter_0_timer();
}



WRITE8_HANDLER( balsente_dac_data_w )
{
	/* LSB or MSB? */
	if (offset & 1)
		dac_value = (dac_value & 0xfc0) | ((data >> 2) & 0x03f);
	else
		dac_value = (dac_value & 0x03f) | ((data << 6) & 0xfc0);

	/* if there are open channels, force the values in */
	if ((chip_select & 0x3f) != 0x3f)
	{
		UINT8 temp = chip_select;
		balsente_chip_select_w(space, 0, 0x3f);
		balsente_chip_select_w(space, 0, temp);
	}
}


WRITE8_HANDLER( balsente_register_addr_w )
{
	dac_register = data & 7;
}



/*************************************
 *
 *  Game-specific handlers
 *
 *************************************/

CUSTOM_INPUT( nstocker_bits_r )
{
	return nstocker_bits;
}


WRITE8_HANDLER( spiker_expand_w )
{
	/* offset 0 is the bit pattern */
	if (offset == 0)
		spiker_expand_bits = data;

	/* offset 1 is the background color (cleared on each read) */
	else if (offset == 1)
		spiker_expand_bgcolor = data;

	/* offset 2 is the color */
	else if (offset == 2)
		spiker_expand_color = data;
}


READ8_HANDLER( spiker_expand_r )
{
	UINT8 left, right;

	/* first rotate each nibble */
	spiker_expand_bits = ((spiker_expand_bits << 1) & 0xee) | ((spiker_expand_bits >> 3) & 0x11);

	/* compute left and right pixels */
	left  = (spiker_expand_bits & 0x10) ? spiker_expand_color : spiker_expand_bgcolor;
	right = (spiker_expand_bits & 0x01) ? spiker_expand_color : spiker_expand_bgcolor;

	/* reset the background color */
	spiker_expand_bgcolor = 0;

	/* return the combined result */
	return (left & 0xf0) | (right & 0x0f);
}


static void update_grudge_steering(running_machine *machine)
{
	UINT8 wheel[3];
	INT8 diff[3];

	/* read the current steering values */
	wheel[0] = input_port_read(machine, "AN0");
	wheel[1] = input_port_read(machine, "AN1");
	wheel[2] = input_port_read(machine, "AN2");

	/* diff the values */
	diff[0] = wheel[0] - grudge_last_steering[0];
	diff[1] = wheel[1] - grudge_last_steering[1];
	diff[2] = wheel[2] - grudge_last_steering[2];

	/* update the last values */
	grudge_last_steering[0] += diff[0];
	grudge_last_steering[1] += diff[1];
	grudge_last_steering[2] += diff[2];

	/* compute the result */
	grudge_steering_result = 0xff;
	if (diff[0])
	{
		grudge_steering_result ^= 0x01;
		if (diff[0] > 0) grudge_steering_result ^= 0x02;
	}
	if (diff[1])
	{
		grudge_steering_result ^= 0x04;
		if (diff[1] > 0) grudge_steering_result ^= 0x08;
	}
	if (diff[2])
	{
		grudge_steering_result ^= 0x10;
		if (diff[2] > 0) grudge_steering_result ^= 0x20;
	}
	logerror("Recomputed steering\n");
}


READ8_HANDLER( grudge_steering_r )
{
	logerror("%04X:grudge_steering_r(@%d)\n", cpu_get_pc(space->cpu), video_screen_get_vpos(space->machine->primary_screen));
	grudge_steering_result |= 0x80;
	return grudge_steering_result;
}



/*************************************
 *
 *  Shrike Avenger CPU memory handlers
 *
 *************************************/

READ8_HANDLER( shrike_shared_6809_r )
{
  UINT16 mem_mask = offset & 1 ? 0xff00 : 0x00ff;

  switch( offset )
  {
    case 6: // return OK for 68k status register until motors hooked up
      return 0;
    default:
      return ( shrike_shared[offset >> 1] & ~mem_mask ) >> ( mem_mask & 8 );
  }
}


WRITE8_HANDLER( shrike_shared_6809_w )
{
  UINT16 mem_mask = offset & 1 ? 0xff00 : 0x00ff;
  shrike_shared[offset >> 1] = ( shrike_shared[offset >> 1] & mem_mask ) | ( data << ( mem_mask & 0x8 ) );
}

// uses movep, so writes even 8 bit addresses to odd 16 bit addresses, reads as 16 bit from odd addresses
// i.e. write 0xdeadbeef to 10000, read 0xde from 10001, 0xad from 10003, 0xbe from 10005...
WRITE16_HANDLER( shrike_io_68k_w )
{
  COMBINE_DATA( &shrike_io[offset] );
}

READ16_HANDLER( shrike_io_68k_r )
{
  return ( shrike_io[offset] & mem_mask ) >> ( 8 & ~mem_mask );
}

