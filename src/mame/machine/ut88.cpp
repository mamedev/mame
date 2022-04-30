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
TIMER_DEVICE_CALLBACK_MEMBER(ut88mini_state::display_timer)
{
	for (u8 i = 0; i < 6; i++)
		m_digits[i] = hex_to_7seg[m_lcd_digit[i]];
}

uint8_t ut88_state::ppi_portb_r()
{
	uint8_t data = 0xff;

	if (BIT(m_keyboard_mask, 0))
		data &= m_io_line0->read();
	if (BIT(m_keyboard_mask, 1))
		data &= m_io_line1->read();
	if (BIT(m_keyboard_mask, 2))
		data &= m_io_line2->read();
	if (BIT(m_keyboard_mask, 3))
		data &= m_io_line3->read();
	if (BIT(m_keyboard_mask, 4))
		data &= m_io_line4->read();
	if (BIT(m_keyboard_mask, 5))
		data &= m_io_line5->read();
	if (BIT(m_keyboard_mask, 6))
		data &= m_io_line6->read();
	if (BIT(m_keyboard_mask, 7))
		data &= m_io_line7->read();

	return data;
}

uint8_t ut88_state::ppi_portc_r()
{
	return m_io_line8->read();
}

void ut88_state::ppi_porta_w(uint8_t data)
{
	m_keyboard_mask = data ^ 0xff;
}

void ut88_state::machine_reset()
{
	m_keyboard_mask = 0;
	address_space &program = m_maincpu->space(AS_PROGRAM);
	program.install_rom(0x0000, 0x07ff, m_rom);   // do it here for F3
	m_rom_shadow_tap.remove();
	m_rom_shadow_tap = program.install_read_tap(
			0xf800, 0xffff,
			"rom_shadow_r",
			[this] (offs_t offset, u8 &data, u8 mem_mask)
			{
				if (!machine().side_effects_disabled())
				{
					// delete this tap
					m_rom_shadow_tap.remove();

					// reinstall RAM over the ROM shadow
					m_maincpu->space(AS_PROGRAM).install_ram(0x0000, 0x07ff, m_ram);
				}
			},
			&m_rom_shadow_tap);
}

void ut88_state::machine_start()
{
	save_item(NAME(m_keyboard_mask));
}

uint8_t ut88_state::keyboard_r(offs_t offset)
{
	return m_ppi->read(offset ^ 0x03);
}


void ut88_state::keyboard_w(offs_t offset, uint8_t data)
{
	m_ppi->write(offset ^ 0x03, data);
}

void ut88_state::sound_w(uint8_t data)
{
	m_dac->write(BIT(data, 0));
	m_cassette->output(BIT(data, 0) ? 1 : -1);
}


uint8_t ut88_common::tape_r()
{
	double level = m_cassette->input();
	return (level <  0) ? 0 : 0xff;
}

uint8_t ut88mini_state::keyboard_r()
{
	// This is real keyboard implementation
	uint8_t *keyrom1 = m_proms->base();
	uint8_t *keyrom2 = m_proms->base()+0x100;

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


void ut88mini_state::led_w(offs_t offset, uint8_t data)
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

void ut88mini_state::machine_start()
{
	m_digits.resolve();
	save_item(NAME(m_lcd_digit));
}

void ut88mini_state::machine_reset()
{
	m_lcd_digit[0] = m_lcd_digit[1] = m_lcd_digit[2] = 0;
	m_lcd_digit[3] = m_lcd_digit[4] = m_lcd_digit[5] = 0;
}
