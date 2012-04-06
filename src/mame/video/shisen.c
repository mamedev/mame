#include "emu.h"
#include "includes/shisen.h"

WRITE8_MEMBER(shisen_state::sichuan2_videoram_w)
{

	m_videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset / 2);
}

WRITE8_MEMBER(shisen_state::sichuan2_bankswitch_w)
{
	int bankaddress;
	int bank;
	UINT8 *RAM = machine().region("maincpu")->base();

	if (data & 0xc0) logerror("bank switch %02x\n",data);

	/* bits 0-2 select ROM bank */
	bankaddress = 0x10000 + (data & 0x07) * 0x4000;
	memory_set_bankptr(machine(), "bank1", &RAM[bankaddress]);

	/* bits 3-5 select gfx bank */
	bank = (data & 0x38) >> 3;

	if (m_gfxbank != bank)
	{
		m_gfxbank = bank;
		machine().tilemap().mark_all_dirty();
	}

	/* bits 6-7 unknown */
}

WRITE8_MEMBER(shisen_state::sichuan2_paletteram_w)
{
	m_paletteram[offset] = data;

	offset &= 0xff;

	palette_set_color_rgb(machine(), offset, pal5bit(m_paletteram[offset + 0x000]), pal5bit(m_paletteram[offset + 0x100]), pal5bit(m_paletteram[offset + 0x200]));
}

static TILE_GET_INFO( get_bg_tile_info )
{
	shisen_state *state = machine.driver_data<shisen_state>();
	int offs = tile_index * 2;
	int code = state->m_videoram[offs] + ((state->m_videoram[offs + 1] & 0x0f) << 8) + (state->m_gfxbank << 12);
	int color = (state->m_videoram[offs + 1] & 0xf0) >> 4;

	SET_TILE_INFO(0, code, color, 0);
}

VIDEO_START( sichuan2 )
{
	shisen_state *state = machine.driver_data<shisen_state>();

	state->m_bg_tilemap = tilemap_create(machine, get_bg_tile_info, tilemap_scan_rows,
		 8, 8, 64, 32);
}

SCREEN_UPDATE_IND16( sichuan2 )
{
	shisen_state *state = screen.machine().driver_data<shisen_state>();

	// on Irem boards, screen flip is handled in both hardware and software.
	// this game doesn't have cocktail mode so if there's software control we don't
	// know where it is mapped.
	flip_screen_set(screen.machine(), ~input_port_read(screen.machine(), "DSW2") & 1);


	state->m_bg_tilemap->draw(bitmap, cliprect, 0, 0);
	return 0;
}
