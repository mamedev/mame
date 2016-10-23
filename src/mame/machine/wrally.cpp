// license:BSD-3-Clause
// copyright-holders:Manuel Abadia, Mike Coates, Nicola Salmoria, Miguel Angel Horna
/***************************************************************************

    World Rally

    Functions to emulate general aspects of the machine (RAM, ROM, interrupts,
    I/O ports)

***************************************************************************/

#include "emu.h"
#include "cpu/mcs51/mcs51.h"
#include "includes/gaelcrpt.h"
#include "includes/wrally.h"


void wrally_state::machine_start()
{
	membank("okibank")->configure_entries(0, 16, memregion("oki")->base(), 0x10000);
}

/***************************************************************************

    World Rally memory handlers

***************************************************************************/

void wrally_state::vram_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask)
{
	data = gaelco_decrypt(space, offset, data, 0x1f, 0x522a);
	COMBINE_DATA(&m_videoram[offset]);

	m_pant[(offset & 0x1fff) >> 12]->mark_tile_dirty(((offset << 1) & 0x1fff) >> 2);
}

void wrally_state::flipscreen_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask)
{
	flip_screen_set(data & 0x01);
}

void wrally_state::okim6295_bankswitch_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if (ACCESSING_BITS_0_7){
		membank("okibank")->set_entry(data & 0x0f);
	}
}

void wrally_state::coin_counter_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask)
{
	machine().bookkeeping().coin_counter_w((offset >> 3) & 0x01, data & 0x01);
}

void wrally_state::coin_lockout_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask)
{
	machine().bookkeeping().coin_lockout_w((offset >> 3) & 0x01, ~data & 0x01);
}
