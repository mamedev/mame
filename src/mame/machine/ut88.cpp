// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/***************************************************************************

        UT88 machine driver by Miodrag Milanovic

        06/03/2008 Preliminary driver.

****************************************************************************/


#include "emu.h"
#include "cpu/i8085/i8085.h"
#include "imagedev/cassette.h"
#include "machine/i8255.h"
#include "includes/ut88.h"

static const uint8_t hex_to_7seg[16] =
{
	0x3F, 0x06, 0x5B, 0x4F,
	0x66, 0x6D, 0x7D, 0x07,
	0x7F, 0x6F, 0x77, 0x7c,
	0x39, 0x5e, 0x79, 0x71
};


/* Driver initialization */
void ut88_state::init_ut88()
{
	/* set initially ROM to be visible on first bank */
	uint8_t *RAM = m_region_maincpu->base();
	memset(RAM,0x0000,0x0800); // make first page empty by default
	m_bank1->configure_entries(1, 2, RAM, 0x0000);
	m_bank1->configure_entries(0, 2, RAM, 0xf800);
}


void ut88_state::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch (id)
	{
	case TIMER_RESET:
		m_bank1->set_entry(0);
		break;
	case TIMER_UPDATE_DISPLAY:
		for (int i=0;i<6;i++)
			output().set_digit_value(i, hex_to_7seg[m_lcd_digit[i]]);
		timer_set(attotime::from_hz(60), TIMER_UPDATE_DISPLAY);
		break;
	default:
		assert_always(false, "Unknown id in ut88_state::device_timer");
	}
}


uint8_t ut88_state::ut88_8255_portb_r(address_space &space, offs_t offset, uint8_t mem_mask)
{
	uint8_t data = 0xff;

	if ( m_keyboard_mask & 0x01 )
		data &= m_io_line0->read();
	if ( m_keyboard_mask & 0x02 )
		data &= m_io_line1->read();
	if ( m_keyboard_mask & 0x04 )
		data &= m_io_line2->read();
	if ( m_keyboard_mask & 0x08 )
		data &= m_io_line3->read();
	if ( m_keyboard_mask & 0x10 )
		data &= m_io_line4->read();
	if ( m_keyboard_mask & 0x20 )
		data &= m_io_line5->read();
	if ( m_keyboard_mask & 0x40 )
		data &= m_io_line6->read();
	if ( m_keyboard_mask & 0x80 )
		data &= m_io_line7->read();

	return data;
}

uint8_t ut88_state::ut88_8255_portc_r(address_space &space, offs_t offset, uint8_t mem_mask)
{
	return m_io_line8->read();
}

void ut88_state::ut88_8255_porta_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask)
{
	m_keyboard_mask = data ^ 0xff;
}

void ut88_state::machine_reset_ut88()
{
	timer_set(attotime::from_usec(10), TIMER_RESET);
	m_bank1->set_entry(1);
	m_keyboard_mask = 0;
}


uint8_t ut88_state::ut88_keyboard_r(address_space &space, offs_t offset, uint8_t mem_mask)
{
	return m_ppi->read(space, offset^0x03);
}


void ut88_state::ut88_keyboard_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask)
{
	m_ppi->write(space, offset^0x03, data);
}

void ut88_state::ut88_sound_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask)
{
	m_dac->write(BIT(data, 0));
	m_cassette->output(BIT(data, 0) ? 1 : -1);
}


uint8_t ut88_state::ut88_tape_r(address_space &space, offs_t offset, uint8_t mem_mask)
{
	double level = m_cassette->input();
	return (level <  0) ? 0 : 0xff;
}

uint8_t ut88_state::ut88mini_keyboard_r(address_space &space, offs_t offset, uint8_t mem_mask)
{
	// This is real keyboard implementation
	uint8_t *keyrom1 = m_region_proms->base();
	uint8_t *keyrom2 = m_region_proms->base()+100;

	uint8_t key = keyrom2[m_io_line1->read()];

	// if keyboard 2nd part returned 0 on 4th bit, output from
	// first part is used

	if (!BIT(key, 3))
		key = keyrom1[m_io_line0->read()];

	// for delete key there is special key producing code 0x80

	key = (BIT(m_io_line2->read(), 7)) ? key : 0x80;

	// If key 0 is pressed its value is 0x10 this is done by additional
	// discrete logic

	key = (BIT(m_io_line0->read(), 0)) ? key : 0x10;
	return key;
}


void ut88_state::ut88mini_write_led(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask)
{
	switch(offset)
	{
		case 0: m_lcd_digit[4] = (data >> 4) & 0x0f;
			m_lcd_digit[5] = data & 0x0f;
			break;
		case 1: m_lcd_digit[2] = (data >> 4) & 0x0f;
			m_lcd_digit[3] = data & 0x0f;
			break;
		case 2: m_lcd_digit[0] = (data >> 4) & 0x0f;
			m_lcd_digit[1] = data & 0x0f;
			break;
		}
}

void ut88_state::init_ut88mini()
{
}

void ut88_state::machine_start_ut88mini()
{
	timer_set(attotime::from_hz(60), TIMER_UPDATE_DISPLAY);
}

void ut88_state::machine_reset_ut88mini()
{
	m_lcd_digit[0] = m_lcd_digit[1] = m_lcd_digit[2] = 0;
	m_lcd_digit[3] = m_lcd_digit[4] = m_lcd_digit[5] = 0;
}
