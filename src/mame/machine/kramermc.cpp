// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/***************************************************************************

        Kramer MC machine driver by Miodrag Milanovic

        13/09/2008 Preliminary driver.

****************************************************************************/


#include "emu.h"
#include "machine/z80pio.h"
#include "includes/kramermc.h"


uint8_t kramermc_state::kramermc_port_a_r(address_space &space, offs_t offset, uint8_t mem_mask)
{
	return 0xff;
}

uint8_t kramermc_state::kramermc_port_b_r(address_space &space, offs_t offset, uint8_t mem_mask)
{
	static const char *const keynames[] = { "LINE0", "LINE1", "LINE2", "LINE3", "LINE4", "LINE5", "LINE6", "LINE7" };

	return ioport(keynames[m_key_row])->read();
}

void kramermc_state::kramermc_port_a_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask)
{
	m_key_row = ((data >> 1) & 0x07);
}

/* Driver initialization */
void kramermc_state::init_kramermc()
{
}

void kramermc_state::machine_reset()
{
	m_key_row = 0;
}
