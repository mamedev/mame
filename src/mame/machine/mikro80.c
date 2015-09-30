// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/***************************************************************************

        Mikro-80 machine driver by Miodrag Milanovic

        10/03/2008 Preliminary driver.

****************************************************************************/


#include "emu.h"
#include "cpu/i8085/i8085.h"
#include "includes/mikro80.h"

/* Driver initialization */
DRIVER_INIT_MEMBER(mikro80_state,mikro80)
{
	/* set initialy ROM to be visible on first bank */
	UINT8 *RAM = m_region_maincpu->base();
	memset(RAM,0x0000,0x0800); // make frist page empty by default
	m_bank1->configure_entries(1, 2, RAM, 0x0000);
	m_bank1->configure_entries(0, 2, RAM, 0xf800);
	m_key_mask = 0x7f;
}

DRIVER_INIT_MEMBER(mikro80_state,radio99)
{
	DRIVER_INIT_CALL(mikro80);
	m_key_mask = 0xff;
}

READ8_MEMBER(mikro80_state::mikro80_8255_portb_r)
{
	UINT8 key = 0xff;
	if ((m_keyboard_mask & 0x01)!=0) { key &= m_io_line0->read(); }
	if ((m_keyboard_mask & 0x02)!=0) { key &= m_io_line1->read(); }
	if ((m_keyboard_mask & 0x04)!=0) { key &= m_io_line2->read(); }
	if ((m_keyboard_mask & 0x08)!=0) { key &= m_io_line3->read(); }
	if ((m_keyboard_mask & 0x10)!=0) { key &= m_io_line4->read(); }
	if ((m_keyboard_mask & 0x20)!=0) { key &= m_io_line5->read(); }
	if ((m_keyboard_mask & 0x40)!=0) { key &= m_io_line6->read(); }
	if ((m_keyboard_mask & 0x80)!=0) { key &= m_io_line7->read(); }
	return key & m_key_mask;
}

READ8_MEMBER(mikro80_state::mikro80_8255_portc_r)
{
	return m_io_line8->read();
}

WRITE8_MEMBER(mikro80_state::mikro80_8255_porta_w)
{
	m_keyboard_mask = data ^ 0xff;
}

WRITE8_MEMBER(mikro80_state::mikro80_8255_portc_w)
{
}

void mikro80_state::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch (id)
	{
	case TIMER_RESET:
		m_bank1->set_entry(0);
		break;
	default:
		assert_always(FALSE, "Unknown id in mikro80_state::device_timer");
	}
}

void mikro80_state::machine_reset()
{
	timer_set(attotime::from_usec(10), TIMER_RESET);
	m_bank1->set_entry(1);
	m_keyboard_mask = 0;
}


READ8_MEMBER(mikro80_state::mikro80_keyboard_r)
{
	return m_ppi8255->read(space, offset^0x03);
}

WRITE8_MEMBER(mikro80_state::mikro80_keyboard_w)
{
	m_ppi8255->write(space, offset^0x03, data);
}


WRITE8_MEMBER(mikro80_state::mikro80_tape_w)
{
	m_cassette->output(data & 0x01 ? 1 : -1);
}


READ8_MEMBER(mikro80_state::mikro80_tape_r)
{
	double level = m_cassette->input();
	if (level <  0) {
			return 0x00;
	}
	return 0xff;
}
