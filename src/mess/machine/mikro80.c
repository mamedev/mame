/***************************************************************************

        Mikro-80 machine driver by Miodrag Milanovic

        10/03/2008 Preliminary driver.

****************************************************************************/


#include "emu.h"
#include "cpu/i8085/i8085.h"
#include "imagedev/cassette.h"
#include "machine/i8255.h"
#include "includes/mikro80.h"

/* Driver initialization */
DRIVER_INIT_MEMBER(mikro80_state,mikro80)
{
	/* set initialy ROM to be visible on first bank */
	UINT8 *RAM = memregion("maincpu")->base();
	memset(RAM,0x0000,0x0800); // make frist page empty by default
	membank("bank1")->configure_entries(1, 2, RAM, 0x0000);
	membank("bank1")->configure_entries(0, 2, RAM, 0xf800);
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
	if ((m_keyboard_mask & 0x01)!=0) { key &= ioport("LINE0")->read(); }
	if ((m_keyboard_mask & 0x02)!=0) { key &= ioport("LINE1")->read(); }
	if ((m_keyboard_mask & 0x04)!=0) { key &= ioport("LINE2")->read(); }
	if ((m_keyboard_mask & 0x08)!=0) { key &= ioport("LINE3")->read(); }
	if ((m_keyboard_mask & 0x10)!=0) { key &= ioport("LINE4")->read(); }
	if ((m_keyboard_mask & 0x20)!=0) { key &= ioport("LINE5")->read(); }
	if ((m_keyboard_mask & 0x40)!=0) { key &= ioport("LINE6")->read(); }
	if ((m_keyboard_mask & 0x80)!=0) { key &= ioport("LINE7")->read(); }
	return key & m_key_mask;
}

READ8_MEMBER(mikro80_state::mikro80_8255_portc_r)
{
	return ioport("LINE8")->read();
}

WRITE8_MEMBER(mikro80_state::mikro80_8255_porta_w)
{
	m_keyboard_mask = data ^ 0xff;
}

WRITE8_MEMBER(mikro80_state::mikro80_8255_portc_w)
{
}

I8255_INTERFACE( mikro80_ppi8255_interface )
{
	DEVCB_NULL,
	DEVCB_DRIVER_MEMBER(mikro80_state, mikro80_8255_porta_w),
	DEVCB_DRIVER_MEMBER(mikro80_state, mikro80_8255_portb_r),
	DEVCB_NULL,
	DEVCB_DRIVER_MEMBER(mikro80_state, mikro80_8255_portc_r),
	DEVCB_NULL,
};


static TIMER_CALLBACK( mikro80_reset )
{
	mikro80_state *state = machine.driver_data<mikro80_state>();
	state->membank("bank1")->set_entry(0);
}

MACHINE_RESET( mikro80 )
{
	mikro80_state *state = machine.driver_data<mikro80_state>();
	machine.scheduler().timer_set(attotime::from_usec(10), FUNC(mikro80_reset));
	state->membank("bank1")->set_entry(1);
	state->m_keyboard_mask = 0;
}


READ8_MEMBER(mikro80_state::mikro80_keyboard_r)
{
	i8255_device *ppi = machine().device<i8255_device>("ppi8255");
	return ppi->read(space, offset^0x03);
}

WRITE8_MEMBER(mikro80_state::mikro80_keyboard_w)
{
	i8255_device *ppi = machine().device<i8255_device>("ppi8255");
	ppi->write(space, offset^0x03, data);
}


WRITE8_MEMBER(mikro80_state::mikro80_tape_w)
{
	machine().device<cassette_image_device>(CASSETTE_TAG)->output(data & 0x01 ? 1 : -1);
}


READ8_MEMBER(mikro80_state::mikro80_tape_r)
{
	double level = machine().device<cassette_image_device>(CASSETTE_TAG)->input();
	if (level <  0) {
			return 0x00;
	}
	return 0xff;
}
