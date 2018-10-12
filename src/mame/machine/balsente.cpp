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


TIMER_DEVICE_CALLBACK_MEMBER(balsente_state::interrupt_timer)
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
		uint8_t tempx, tempy;

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
	/* create the polynomial tables */
	poly17_init();

	m_acia->write_cts(0);
	m_acia->write_dcd(0);
	m_audiouart->write_cts(0);
	m_audiouart->write_dcd(0);

	save_item(NAME(m_counter_control));
	save_item(NAME(m_counter_0_ff));
	save_item(NAME(m_counter_0_out));
	save_item(NAME(m_counter_0_timer_active));

	save_item(NAME(m_analog_input_data));
	save_item(NAME(m_adc_value));

	save_item(NAME(m_dac_value));
	save_item(NAME(m_dac_register));
	save_item(NAME(m_chip_select));

	save_item(NAME(m_uint));

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
	int numbanks;

	/* reset the manual counter 0 clock */
	m_counter_control = 0x00;
	m_counter_0_ff = false;
	m_counter_0_out = false;
	m_counter_0_timer_active = false;
	m_audiocpu->set_input_line(INPUT_LINE_NMI, CLEAR_LINE);

	/* reset the ADC states */
	m_adc_value = 0;

	/* reset the CEM3394 I/O states */
	m_dac_value = 0;
	m_dac_register = 0;
	m_chip_select = 0x3f;

	/* reset game-specific states */
	m_grudge_steering_result = 0;

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
	uint32_t i, x = 0;
	uint8_t *p, *r;

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
	uint32_t step = (100000 << 14) / cem3394_device::SAMPLE_RATE;
	uint32_t noise_counter = m_noise_position[chip];

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

WRITE8_MEMBER(balsente_state::random_reset_w)
{
	/* reset random number generator */
}


READ8_MEMBER(balsente_state::random_num_r)
{
	uint32_t cc;

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

WRITE8_MEMBER(balsente_state::rombank_select_w)
{
	/* the bank number comes from bits 4-6 */
	membank("bank1")->set_entry((data >> 4) & 7);
	membank("bank2")->set_entry((data >> 4) & 7);
}


WRITE8_MEMBER(balsente_state::rombank2_select_w)
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

WRITE_LINE_MEMBER(balsente_state::out0_w)
{
//      output().set_led_value(0, state);
}

WRITE_LINE_MEMBER(balsente_state::out1_w)
{
//      output().set_led_value(1, state);
}

WRITE_LINE_MEMBER(balsente_state::out2_w)
{
//      output().set_led_value(2, state);
}

WRITE_LINE_MEMBER(balsente_state::out3_w)
{
//      output().set_led_value(3, state);
}

WRITE_LINE_MEMBER(balsente_state::out4_w)
{
//      output().set_led_value(4, state);
}

WRITE_LINE_MEMBER(balsente_state::out5_w)
{
//      output().set_led_value(5, state);
}

WRITE_LINE_MEMBER(balsente_state::out6_w)
{
//      output().set_led_value(6, state);
}

WRITE_LINE_MEMBER(balsente_state::nvrecall_w)
{
	m_novram[0]->recall(!state);
	m_novram[1]->recall(!state);
}

READ8_MEMBER(balsente_state::novram_8bit_r)
{
	return (m_novram[0]->read(space, offset) & 0x0f) | (m_novram[1]->read(space, offset) << 4);
}

WRITE8_MEMBER(balsente_state::novram_8bit_w)
{
	m_novram[0]->write(space, offset, data & 0x0f);
	m_novram[1]->write(space, offset, data >> 4);
}



/*************************************
 *
 *  6850 UART communications
 *
 *************************************/

WRITE8_MEMBER(balsente_state::acia_w)
{
	// Ugly workaround: suppress soft reset command in order to avert race condition
	m_acia->write(space, offset, (BIT(offset, 0) && data == 0xe0) ? 0 : data);
}

WRITE_LINE_MEMBER(balsente_state::uint_propagate_w)
{
	if (state && BIT(m_counter_control, 5))
		m_audiocpu->set_input_line(INPUT_LINE_NMI, m_uint ? ASSERT_LINE : CLEAR_LINE);
}



/*************************************
 *
 *  ADC handlers
 *
 *************************************/

INTERRUPT_GEN_MEMBER(balsente_state::update_analog_inputs)
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


READ8_MEMBER(balsente_state::adc_data_r)
{
	/* just return the last value read */
	return m_adc_value;
}


WRITE8_MEMBER(balsente_state::adc_select_w)
{
	/* set a timer to go off and read the value after 50us */
	/* it's important that we do this for Mini Golf */
	logerror("adc_select %d\n", offset & 7);
	machine().scheduler().timer_set(attotime::from_usec(50), timer_expired_delegate(FUNC(balsente_state::adc_finished),this), offset & 7);
}

READ8_MEMBER(balsente_state::teamht_extra_r)
{
	return m_teamht_input;
}


WRITE8_MEMBER(balsente_state::teamht_multiplex_select_w)
{
	logerror("multiplex_select %d\n", offset & 7);
	switch (offset & 7)
	{
	case 0x04: m_teamht_input = ioport("EX0")->read(); break;
	case 0x05: m_teamht_input = ioport("EX1")->read(); break;
	case 0x06: m_teamht_input = ioport("EX2")->read(); break;
	case 0x07: m_teamht_input = ioport("EX3")->read(); break;

	default:
		logerror("(unhandled)\n");
		break;

	}

}




/*************************************
 *
 *  Sound CPU counter 0 emulation
 *
 *************************************/

WRITE_LINE_MEMBER(balsente_state::counter_0_set_out)
{
	/* OUT on counter 0 is hooked to the GATE line on counter 1 through an inverter */
	m_pit->write_gate1(!state);

	/* remember the out state */
	m_counter_0_out = state;
}


WRITE_LINE_MEMBER(balsente_state::set_counter_0_ff)
{
	/* the flip/flop output is inverted, so if we went high to low, that's a clock */
	m_pit->write_clk0(!state);

	/* remember the new state */
	m_counter_0_ff = state;
}


TIMER_DEVICE_CALLBACK_MEMBER(balsente_state::clock_counter_0_ff)
{
	/* clock the D value through the flip-flop */
	set_counter_0_ff(BIT(m_counter_control, 3));
}


void balsente_state::update_counter_0_timer()
{
	double maxfreq = 0.0;
	int i;

	/* if there's already a timer, remove it */
	if (m_counter_0_timer_active)
		m_counter_0_timer->reset();
	m_counter_0_timer_active = false;

	/* find the counter with the maximum frequency */
	/* this is used to calibrate the timers at startup */
	for (i = 0; i < 6; i++)
		if (m_cem_device[i]->get_parameter(cem3394_device::FINAL_GAIN) < 10.0)
		{
			double tempfreq;

			/* if the filter resonance is high, then they're calibrating the filter frequency */
			if (m_cem_device[i]->get_parameter(cem3394_device::FILTER_RESONANCE) > 0.9)
				tempfreq = m_cem_device[i]->get_parameter(cem3394_device::FILTER_FREQENCY);

			/* otherwise, they're calibrating the VCO frequency */
			else
				tempfreq = m_cem_device[i]->get_parameter(cem3394_device::VCO_FREQUENCY);

			if (tempfreq > maxfreq) maxfreq = tempfreq;
		}

	/* reprime the timer */
	if (maxfreq > 0.0)
	{
		m_counter_0_timer_active = true;
		m_counter_0_timer->adjust(attotime::from_hz(maxfreq), 0, attotime::from_hz(maxfreq));
	}
}



/*************************************
 *
 *  Sound CPU counter handlers
 *
 *************************************/

READ8_MEMBER(balsente_state::counter_state_r)
{
	/* bit D0 is the inverse of the flip-flop state */
	int result = !m_counter_0_ff;

	/* bit D1 is the OUT value from counter 0 */
	if (m_counter_0_out) result |= 0x02;

	return result;
}


WRITE8_MEMBER(balsente_state::counter_control_w)
{
	uint8_t diff_counter_control = m_counter_control ^ data;

	/* set the new global value */
	m_counter_control = data;

	/* bit D0 enables/disables audio */
	if (BIT(diff_counter_control, 0))
	{
		for (auto & elem : m_cem_device)
			elem->set_output_gain(0, BIT(data, 0) ? 1.0 : 0);
	}

	/* bit D1 is hooked to counter 0's gate */
	if (BIT(diff_counter_control, 1))
	{
		/* if we gate on, start a pulsing timer to clock it */
		if (BIT(data, 1) && !m_counter_0_timer_active)
		{
			update_counter_0_timer();
		}

		/* if we gate off, remove the timer */
		else if (!BIT(data, 1) && m_counter_0_timer_active)
		{
			m_counter_0_timer->reset();
			m_counter_0_timer_active = false;
		}
	}

	/* set the actual gate */
	m_pit->write_gate0(BIT(data, 1));

	/* bits D2 and D4 control the clear/reset flags on the flip-flop that feeds counter 0 */
	if (!BIT(data, 4))
		set_counter_0_ff(0);
	else if (!BIT(data, 2))
		set_counter_0_ff(1);

	/* bit 5 clears the NMI interrupt */
	if (BIT(diff_counter_control, 5) && !BIT(data, 5))
		m_audiocpu->set_input_line(INPUT_LINE_NMI, CLEAR_LINE);
}



/*************************************
 *
 *  CEM3394 Interfaces
 *
 *************************************/

WRITE8_MEMBER(balsente_state::chip_select_w)
{
	static constexpr uint8_t register_map[8] =
	{
		cem3394_device::VCO_FREQUENCY,
		cem3394_device::FINAL_GAIN,
		cem3394_device::FILTER_RESONANCE,
		cem3394_device::FILTER_FREQENCY,
		cem3394_device::MIXER_BALANCE,
		cem3394_device::MODULATION_AMOUNT,
		cem3394_device::PULSE_WIDTH,
		cem3394_device::WAVE_SELECT
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
				logerror("s%04X:   CEM#%d:%s=%f\n", m_audiocpu->pcbase(), i, names[m_dac_register], voltage);
			}
#endif
		}

	/* if a timer for counter 0 is running, recompute */
	if (m_counter_0_timer_active)
		update_counter_0_timer();
}



WRITE8_MEMBER(balsente_state::dac_data_w)
{
	/* LSB or MSB? */
	if (offset & 1)
		m_dac_value = (m_dac_value & 0xfc0) | ((data >> 2) & 0x03f);
	else
		m_dac_value = (m_dac_value & 0x03f) | ((data << 6) & 0xfc0);

	/* if there are open channels, force the values in */
	if ((m_chip_select & 0x3f) != 0x3f)
	{
		uint8_t temp = m_chip_select;
		chip_select_w(space, 0, 0x3f);
		chip_select_w(space, 0, temp);
	}
}


WRITE8_MEMBER(balsente_state::register_addr_w)
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
	uint8_t left, right;

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
	uint8_t wheel[3];
	int8_t diff[3];

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
	logerror("%s:grudge_steering_r(@%d)\n", machine().describe_context(), m_screen->vpos());
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
	uint16_t mem_mask_int = offset & 1 ? 0xff00 : 0x00ff;

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
	uint16_t mem_mask_int = offset & 1 ? 0xff00 : 0x00ff;
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
