#include "emu.h"
#include "video/deco16ic.h"
#include "includes/pktgaldx.h"
#include "video/decospr.h"

/* Video on the orginal */

SCREEN_UPDATE_IND16( pktgaldx )
{
	pktgaldx_state *state = screen.machine().driver_data<pktgaldx_state>();
	UINT16 flip = deco16ic_pf_control_r(state->m_deco_tilegen1, 0, 0xffff);

	state->flip_screen_set(BIT(flip, 7));
	deco16ic_pf_update(state->m_deco_tilegen1, state->m_pf1_rowscroll, state->m_pf2_rowscroll);

	bitmap.fill(0, cliprect); /* not Confirmed */
	screen.machine().priority_bitmap.fill(0);

	deco16ic_tilemap_2_draw(state->m_deco_tilegen1, bitmap, cliprect, 0, 0);
	screen.machine().device<decospr_device>("spritegen")->draw_sprites(bitmap, cliprect, state->m_spriteram, 0x400, true);
	deco16ic_tilemap_1_draw(state->m_deco_tilegen1, bitmap, cliprect, 0, 0);
	return 0;
}

/* Video for the bootleg */

SCREEN_UPDATE_IND16( pktgaldb )
{
	pktgaldx_state *state = screen.machine().driver_data<pktgaldx_state>();
	int x, y;
	int offset = 0;
	int tileno;
	int colour;

	bitmap.fill(get_black_pen(screen.machine()), cliprect);

	/* the bootleg seems to treat the tilemaps as sprites */
	for (offset = 0; offset < 0x1600 / 2; offset += 8)
	{
		tileno = state->m_pktgaldb_sprites[offset + 3] | (state->m_pktgaldb_sprites[offset + 2] << 16);
		colour = state->m_pktgaldb_sprites[offset + 1] >> 1;
		x = state->m_pktgaldb_sprites[offset + 0];
		y = state->m_pktgaldb_sprites[offset + 4];

		x -= 0xc2;
		y &= 0x1ff;
		y -= 8;

		drawgfx_transpen(bitmap, cliprect, screen.machine().gfx[0], tileno ^ 0x1000, colour, 0, 0, x, y, 0);
	}

	for (offset = 0x1600/2; offset < 0x2000 / 2; offset += 8)
	{
		tileno = state->m_pktgaldb_sprites[offset + 3] | (state->m_pktgaldb_sprites[offset + 2] << 16);
		colour = state->m_pktgaldb_sprites[offset + 1] >> 1;
		x = state->m_pktgaldb_sprites[offset + 0] & 0x1ff;
		y = state->m_pktgaldb_sprites[offset + 4] & 0x0ff;

		x -= 0xc2;
		y &= 0x1ff;
		y -= 8;

		drawgfx_transpen(bitmap, cliprect, screen.machine().gfx[0], tileno ^ 0x4000, colour, 0, 0, x, y, 0);
	}

	for (offset = 0x2000/2; offset < 0x4000 / 2; offset += 8)
	{
		tileno = state->m_pktgaldb_sprites[offset + 3] | (state->m_pktgaldb_sprites[offset + 2] << 16);
		colour = state->m_pktgaldb_sprites[offset + 1] >> 1;
		x = state->m_pktgaldb_sprites[offset + 0] & 0x1ff;
		y = state->m_pktgaldb_sprites[offset + 4] & 0x0ff;

		x -= 0xc2;
		y &= 0x1ff;
		y -= 8;

		drawgfx_transpen(bitmap, cliprect, screen.machine().gfx[0], tileno ^ 0x3000, colour, 0, 0, x, y, 0);
	}

	return 0;
}
