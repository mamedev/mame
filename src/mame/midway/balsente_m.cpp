// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    Bally/Sente SAC-1 system

    driver by Aaron Giles

***************************************************************************/

#include "emu.h"
#include "cpu/m6809/m6809.h"
#include "balsente.h"


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
	m_irq_off_timer->adjust(m_screen->time_until_pos(param, BALSENTE_HBSTART));

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
	if (m_acia.found())
	{
		m_acia->write_cts(0);
		m_acia->write_dcd(0);
	}

	save_item(NAME(m_nstocker_bits));
	save_item(NAME(m_spiker_expand_color));
	save_item(NAME(m_spiker_expand_bgcolor));
	save_item(NAME(m_spiker_expand_bits));
	save_item(NAME(m_grudge_steering_result));
	save_item(NAME(m_grudge_last_steering));

	m_irq_off_timer = timer_alloc(FUNC(balsente_state::irq_off), this);
	m_adc_timer = timer_alloc(FUNC(balsente_state::adc_finished), this);
}


void balsente_state::machine_reset()
{
	/* create the polynomial tables */
	poly17_init();

	/* reset the ADC states */
	m_adc_value = 0;

	/* reset game-specific states */
	m_grudge_steering_result = 0;

	/* point the banks to bank 0 */
	m_bankab->set_entry(0);
	m_bankcd->set_entry(0);
	m_bankef->set_entry(0);
	m_maincpu->reset();

	/* start a timer to generate interrupts */
	m_scanline_timer->adjust(m_screen->time_until_pos(0));

	m_irq_off_timer->adjust(attotime::never);
	m_adc_timer->adjust(attotime::never);
}



/*************************************
 *
 *  Hardware random numbers
 *
 *  NOTE: this is stolen straight from
 *          POKEY.c
 *
 *************************************/

void balsente_state::poly17_init()
{
	uint32_t i, x = 0;
	uint8_t *r;

	/* allocate memory */
	r = m_rand17;

	/* generate the polynomial */
	for (i = 0; i < POLY17_SIZE; i++)
	{
		/* store new values */
		*r++ = x >> 3;

		/* calculate next bit */
		x = ((x << POLY17_SHL) + (x >> POLY17_SHR) + POLY17_ADD) & POLY17_SIZE;
	}
}

void balsente_state::random_reset_w(uint8_t data)
{
	/* reset random number generator */
}


uint8_t balsente_state::random_num_r()
{
	/* CPU runs at 1.25MHz, noise source at 100kHz --> multiply by 12.5 */
	uint32_t cc = m_maincpu->total_cycles();

	/* 12.5 = 8 + 4 + 0.5 */
	cc = (cc << 3) + (cc << 2) + (cc >> 1);
	return m_rand17[cc & POLY17_SIZE];
}



/*************************************
 *
 *  ROM banking
 *
 *************************************/

void balsente_state::rombank_select_w(uint8_t data)
{
	/* the bank number comes from bits 4-6 */
	m_bankab->set_entry((data >> 4) & 7);
	m_bankcd->set_entry((data >> 4) & 7);
	m_bankef->set_entry(0);
}


void balsente_state::rombank2_select_w(uint8_t data)
{
	/* Night Stocker and Name that Tune only so far.... */
	int bank = data & 7;

	/* top bit controls which half of the ROMs to use (Name that Tune only) */
	if (m_mainrom->bytes() > 0x20000) bank |= (data >> 4) & 8;

	/* when they set the AB bank, it appears as though the CD bank is reset */
	if (data & 0x20)
	{
		m_bankab->set_entry(bank);
		m_bankcd->set_entry(6);
		m_bankef->set_entry(0);
	}

	/* set both banks */
	else
	{
		m_bankab->set_entry(bank);
		m_bankcd->set_entry(bank);
		m_bankef->set_entry(BIT(bank, 3));
	}
}



/*************************************
 *
 *  Special outputs
 *
 *************************************/

void balsente_state::out0_w(int state)
{
//      output().set_led_value(0, state);
}

void balsente_state::out1_w(int state)
{
//      output().set_led_value(1, state);
}

void balsente_state::out2_w(int state)
{
//      output().set_led_value(2, state);
}

void balsente_state::out3_w(int state)
{
//      output().set_led_value(3, state);
}

void balsente_state::out4_w(int state)
{
//      output().set_led_value(4, state);
}

void balsente_state::out5_w(int state)
{
//      output().set_led_value(5, state);
}

void balsente_state::out6_w(int state)
{
//      output().set_led_value(6, state);
}

void balsente_state::nvrecall_w(int state)
{
	m_novram[0]->recall(!state);
	m_novram[1]->recall(!state);
}

uint8_t balsente_state::novram_8bit_r(address_space &space, offs_t offset)
{
	return (m_novram[0]->read(space, offset) & 0x0f) | (m_novram[1]->read(space, offset) << 4);
}

void balsente_state::novram_8bit_w(offs_t offset, uint8_t data)
{
	m_novram[0]->write(offset, data & 0x0f);
	m_novram[1]->write(offset, data >> 4);
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


uint8_t balsente_state::adc_data_r()
{
	/* just return the last value read */
	return m_adc_value;
}


void balsente_state::adc_select_w(offs_t offset, uint8_t data)
{
	/* set a timer to go off and read the value after 50us */
	/* it's important that we do this for Mini Golf */
	logerror("adc_select %d\n", offset & 7);
	m_adc_timer->adjust(attotime::from_usec(50), offset & 7);
}

uint8_t balsente_state::teamht_extra_r()
{
	return m_teamht_input;
}


void balsente_state::teamht_multiplex_select_w(offs_t offset, uint8_t data)
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
 *  Game-specific handlers
 *
 *************************************/

ioport_value balsente_state::nstocker_bits_r()
{
	return m_nstocker_bits;
}


void balsente_state::spiker_expand_w(offs_t offset, uint8_t data)
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

uint8_t balsente_state::spiker_expand_r()
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


uint8_t balsente_state::grudge_steering_r()
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

uint8_t balsente_state::shrike_shared_6809_r(offs_t offset)
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


void balsente_state::shrike_shared_6809_w(offs_t offset, uint8_t data)
{
	uint16_t mem_mask_int = offset & 1 ? 0xff00 : 0x00ff;
	m_shrike_shared[offset >> 1] = ( m_shrike_shared[offset >> 1] & mem_mask_int ) | ( data << ( mem_mask_int & 0x8 ) );
}

// uses movep, so writes even 8 bit addresses to odd 16 bit addresses, reads as 16 bit from odd addresses
// i.e. write 0xdeadbeef to 10000, read 0xde from 10001, 0xad from 10003, 0xbe from 10005...
void balsente_state::shrike_io_68k_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA( &m_shrike_io[offset] );
}

uint16_t balsente_state::shrike_io_68k_r(offs_t offset, uint16_t mem_mask)
{
	return ( m_shrike_io[offset] & mem_mask ) >> ( 8 & ~mem_mask );
}
