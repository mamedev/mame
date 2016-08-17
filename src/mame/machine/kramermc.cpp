// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/***************************************************************************

        Kramer MC machine driver by Miodrag Milanovic

        13/09/2008 Preliminary driver.

****************************************************************************/


#include "emu.h"
#include "machine/z80pio.h"
#include "includes/kramermc.h"


READ8_MEMBER(kramermc_state::kramermc_port_a_r)
{
	return 0xff;
}

READ8_MEMBER(kramermc_state::kramermc_port_b_r)
{
	static const char *const keynames[] = { "LINE0", "LINE1", "LINE2", "LINE3", "LINE4", "LINE5", "LINE6", "LINE7" };

	return ioport(keynames[m_key_row])->read();
}

WRITE8_MEMBER(kramermc_state::kramermc_port_a_w)
{
	m_key_row = ((data >> 1) & 0x07);
}

/* Driver initialization */
DRIVER_INIT_MEMBER(kramermc_state,kramermc)
{
}

void kramermc_state::machine_reset()
{
	m_key_row = 0;
}
