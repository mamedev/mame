/***************************************************************************

  video.c

  Functions to emulate the video hardware of the machine.

***************************************************************************/
#include "emu.h"
#include "includes/ladyfrog.h"


WRITE8_MEMBER(ladyfrog_state::ladyfrog_spriteram_w)
{
	m_spriteram[offset] = data;
}

READ8_MEMBER(ladyfrog_state::ladyfrog_spriteram_r)
{
	return m_spriteram[offset];
}

static TILE_GET_INFO( get_tile_info )
{
	ladyfrog_state *state = machine.driver_data<ladyfrog_state>();
	int pal = state->m_videoram[tile_index * 2 + 1] & 0x0f;
	int tile = state->m_videoram[tile_index * 2] + ((state->m_videoram[tile_index * 2 + 1] & 0xc0) << 2)+ ((state->m_videoram[tile_index * 2 + 1] & 0x30) << 6);
	SET_TILE_INFO(
			0,
			tile + 0x1000 * state->m_tilebank,
			pal,TILE_FLIPY
			);
}

WRITE8_MEMBER(ladyfrog_state::ladyfrog_videoram_w)
{
	m_videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset >> 1);
}

READ8_MEMBER(ladyfrog_state::ladyfrog_videoram_r)
{
	return m_videoram[offset];
}

WRITE8_MEMBER(ladyfrog_state::ladyfrog_palette_w)
{

	if (offset & 0x100)
		paletteram_xxxxBBBBGGGGRRRR_split2_w(space, (offset & 0xff) + (m_palette_bank << 8), data);
	else
		paletteram_xxxxBBBBGGGGRRRR_split1_w(space, (offset & 0xff) + (m_palette_bank << 8), data);
}

READ8_MEMBER(ladyfrog_state::ladyfrog_palette_r)
{

	if (offset & 0x100)
		return m_generic_paletteram2_8[(offset & 0xff) + (m_palette_bank << 8)];
	else
		return m_generic_paletteram_8[(offset & 0xff) + (m_palette_bank << 8)];
}

WRITE8_MEMBER(ladyfrog_state::ladyfrog_gfxctrl_w)
{
	m_palette_bank = (data & 0x20) >> 5;
}

WRITE8_MEMBER(ladyfrog_state::ladyfrog_gfxctrl2_w)
{
	m_tilebank = ((data & 0x18) >> 3) ^ 3;
	m_bg_tilemap->mark_all_dirty();
}


#ifdef UNUSED_FUNCTION
int gfxctrl;

READ8_MEMBER(ladyfrog_state::ladyfrog_gfxctrl_r)
{
	return gfxctrl;
}
#endif

READ8_MEMBER(ladyfrog_state::ladyfrog_scrlram_r)
{
	return m_scrlram[offset];
}

WRITE8_MEMBER(ladyfrog_state::ladyfrog_scrlram_w)
{

	m_scrlram[offset] = data;
	m_bg_tilemap->set_scrolly(offset, data);
}

static void draw_sprites( running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	ladyfrog_state *state = machine.driver_data<ladyfrog_state>();
	int i;
	for (i = 0; i < 0x20; i++)
	{
		int pr = state->m_spriteram[0x9f - i];
		int offs = (pr & 0x1f) * 4;
		{
			int code, sx, sy, flipx, flipy, pal;
			code = state->m_spriteram[offs + 2] + ((state->m_spriteram[offs + 1] & 0x10) << 4) + state->m_spritetilebase;
			pal = state->m_spriteram[offs + 1] & 0x0f;
			sx = state->m_spriteram[offs + 3];
			sy = 238 - state->m_spriteram[offs + 0];
			flipx = ((state->m_spriteram[offs + 1] & 0x40)>>6);
			flipy = ((state->m_spriteram[offs + 1] & 0x80)>>7);
			drawgfx_transpen(bitmap,cliprect,machine.gfx[1],
					code,
					pal,
					flipx,flipy,
					sx,sy,15);

			if (state->m_spriteram[offs + 3] > 240)
			{
				sx = (state->m_spriteram[offs + 3] - 256);
				drawgfx_transpen(bitmap,cliprect,machine.gfx[1],
        				code,
				        pal,
				        flipx,flipy,
					      sx,sy,15);
			}
		}
	}
}

static VIDEO_START( ladyfrog_common )
{
	ladyfrog_state *state = machine.driver_data<ladyfrog_state>();

	state->m_spriteram = auto_alloc_array(machine, UINT8, 160);
	state->m_bg_tilemap = tilemap_create(machine, get_tile_info, tilemap_scan_rows, 8, 8, 32, 32);

	state->m_generic_paletteram_8.allocate(0x200);
	state->m_generic_paletteram2_8.allocate(0x200);
	state->m_bg_tilemap->set_scroll_cols(32);
	state->m_bg_tilemap->set_scrolldy(15, 15);

	state->save_pointer(NAME(state->m_spriteram), 160);
}

VIDEO_START( ladyfrog )
{
	ladyfrog_state *state = machine.driver_data<ladyfrog_state>();

	// weird, there are sprite tiles at 0x000 and 0x400, but they don't contain all the sprites!
	state->m_spritetilebase = 0x800;
	VIDEO_START_CALL(ladyfrog_common);
}

VIDEO_START( toucheme )
{
	ladyfrog_state *state = machine.driver_data<ladyfrog_state>();

	state->m_spritetilebase = 0x000;
	VIDEO_START_CALL(ladyfrog_common);
}


SCREEN_UPDATE_IND16( ladyfrog )
{
	ladyfrog_state *state = screen.machine().driver_data<ladyfrog_state>();

	state->m_bg_tilemap->draw(bitmap, cliprect, 0, 0);
	draw_sprites(screen.machine(), bitmap, cliprect);
	return 0;
}
