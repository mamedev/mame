// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/***************************************************************************

        UT88 machine driver by Miodrag Milanovic

        06/03/2008 Preliminary driver.

****************************************************************************/


#include "emu.h"
#include "cpu/i8085/i8085.h"
#include "sound/dac.h"
#include "imagedev/cassette.h"
#include "machine/i8255.h"
#include "includes/ut88.h"

static const UINT8 hex_to_7seg[16] =
{
	0x3F, 0x06, 0x5B, 0x4F,
	0x66, 0x6D, 0x7D, 0x07,
	0x7F, 0x6F, 0x77, 0x7c,
	0x39, 0x5e, 0x79, 0x71
};


/* Driver initialization */
DRIVER_INIT_MEMBER(ut88_state,ut88)
{
	/* set initially ROM to be visible on first bank */
	UINT8 *RAM = m_region_maincpu->base();
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
			output_set_digit_value(i, hex_to_7seg[m_lcd_digit[i]]);
		timer_set(attotime::from_hz(60), TIMER_UPDATE_DISPLAY);
		break;
	default:
		assert_always(FALSE, "Unknown id in ut88_state::device_timer");
	}
}


READ8_MEMBER( ut88_state::ut88_8255_portb_r )
{
	UINT8 data = 0xff;

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

READ8_MEMBER( ut88_state::ut88_8255_portc_r )
{
	return m_io_line8->read();
}

WRITE8_MEMBER( ut88_state::ut88_8255_porta_w )
{
	m_keyboard_mask = data ^ 0xff;
}

MACHINE_RESET_MEMBER(ut88_state,ut88)
{
	timer_set(attotime::from_usec(10), TIMER_RESET);
	m_bank1->set_entry(1);
	m_keyboard_mask = 0;
}


READ8_MEMBER( ut88_state::ut88_keyboard_r )
{
	return m_ppi->read(space, offset^0x03);
}


WRITE8_MEMBER( ut88_state::ut88_keyboard_w )
{
	m_ppi->write(space, offset^0x03, data);
}

WRITE8_MEMBER( ut88_state::ut88_sound_w )
{
	m_dac->write_unsigned8(data); //beeper
	m_cassette->output(BIT(data, 0) ? 1 : -1);
}


READ8_MEMBER( ut88_state::ut88_tape_r )
{
	double level = m_cassette->input();
	return (level <  0) ? 0 : 0xff;
}

READ8_MEMBER( ut88_state::ut88mini_keyboard_r )
{
	// This is real keyboard implementation
	UINT8 *keyrom1 = m_region_proms->base();
	UINT8 *keyrom2 = m_region_proms->base()+100;

	UINT8 key = keyrom2[m_io_line1->read()];

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


WRITE8_MEMBER( ut88_state::ut88mini_write_led )
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

DRIVER_INIT_MEMBER(ut88_state,ut88mini)
{
}

MACHINE_START_MEMBER(ut88_state,ut88mini)
{
	timer_set(attotime::from_hz(60), TIMER_UPDATE_DISPLAY);
}

MACHINE_RESET_MEMBER(ut88_state,ut88mini)
{
	m_lcd_digit[0] = m_lcd_digit[1] = m_lcd_digit[2] = 0;
	m_lcd_digit[3] = m_lcd_digit[4] = m_lcd_digit[5] = 0;
}
