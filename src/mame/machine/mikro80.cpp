// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/***************************************************************************

        Mikro-80 machine driver by Miodrag Milanovic

        10/03/2008 Preliminary driver.

****************************************************************************/


#include "emu.h"
#include "includes/mikro80.h"

/* Driver initialization */
void mikro80_state::init_mikro80()
{
	m_key_mask = 0x7f;
}

void mikro80_state::init_radio99()
{
	m_key_mask = 0xff;
}

u8 mikro80_state::portb_r()
{
	u8 key = 0xff;
	for (u8 i = 0; i < 8; i++)
		if (BIT(m_keyboard_mask, i))
			key &= m_io_keyboard[i]->read();

	return key & m_key_mask;
}

u8 mikro80_state::portc_r()
{
	return m_io_keyboard[8]->read();
}

u8 mikro80_state::kristall2_portc_r()
{
	return (m_io_keyboard[8]->read() & 0xfe) | ((m_cassette->input() < 0.04) ? 1 : 0);
}

void mikro80_state::porta_w(u8 data)
{
	m_keyboard_mask = data ^ 0xff;
}

void mikro80_state::portc_w(u8 data)
{
	m_cassette->output(BIT(data, 7) ? 1.0 : -1.0);   // for Kristall2 only
}

void mikro80_state::machine_start()
{
	save_item(NAME(m_keyboard_mask));
	save_item(NAME(m_key_mask));
}

void mikro80_state::machine_reset()
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

void mikro80_state::tape_w(u8 data)
{
	// TODO: this is incorrect, to be fixed when the CMT schematic can be found
	m_cassette->output(BIT(data, 0) ? 1.0 : -1.0);
}


u8 mikro80_state::tape_r()
{
	return (m_cassette->input() < 0.04) ? 0xff : 0;
}

void mikro80_state::sound_w(u8 data)
{
	m_dac->write(BIT(data, 1));
}
