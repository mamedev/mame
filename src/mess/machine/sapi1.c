/***************************************************************************

        SAPI-1 driver by Miodrag Milanovic

        09/09/2008 Preliminary driver.

****************************************************************************/

#include "includes/sapi1.h"


/* Driver initialization */
DRIVER_INIT_MEMBER(sapi1_state,sapi1)
{
}

MACHINE_RESET( sapi1 )
{
	sapi1_state *state = machine.driver_data<sapi1_state>();
	state->m_keyboard_mask = 0;
}

MACHINE_START(sapi1)
{
}

READ8_MEMBER( sapi1_state::sapi1_keyboard_r )
{
	UINT8 key = 0xff;
	if (BIT(m_keyboard_mask, 0)) { key &= ioport("LINE0")->read(); }
	if (BIT(m_keyboard_mask, 1)) { key &= ioport("LINE1")->read(); }
	if (BIT(m_keyboard_mask, 2)) { key &= ioport("LINE2")->read(); }
	if (BIT(m_keyboard_mask, 3)) { key &= ioport("LINE3")->read(); }
	if (BIT(m_keyboard_mask, 4)) { key &= ioport("LINE4")->read(); }
	return key;
}

WRITE8_MEMBER( sapi1_state::sapi1_keyboard_w )
{
	m_keyboard_mask = (data ^ 0xff ) & 0x1f;
}


MACHINE_RESET( sapizps3 )
{
	sapi1_state *state = machine.driver_data<sapi1_state>();
	state->m_keyboard_mask = 0;
	state->membank("bank1")->set_entry(1);
}

DRIVER_INIT_MEMBER(sapi1_state,sapizps3)
{
	UINT8 *RAM = memregion("maincpu")->base();
	membank("bank1")->configure_entries(0, 2, &RAM[0x0000], 0xf800);
}
