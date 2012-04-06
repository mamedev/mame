#include "emu.h"
#include "video/konicdev.h"
#include "includes/tail2nos.h"


#define TOTAL_CHARS 0x400

/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

static TILE_GET_INFO( get_tile_info )
{
	tail2nos_state *state = machine.driver_data<tail2nos_state>();
	UINT16 code = state->m_bgvideoram[tile_index];
	SET_TILE_INFO(
			0,
			(code & 0x1fff) + (state->m_charbank << 13),
			((code & 0xe000) >> 13) + state->m_charpalette * 16,
			0);
}


/***************************************************************************

  Callbacks for the K051316

***************************************************************************/

void tail2nos_zoom_callback( running_machine &machine, int *code, int *color, int *flags )
{
	*code |= ((*color & 0x03) << 8);
	*color = 32 + ((*color & 0x38) >> 3);
}

/***************************************************************************

    Start the video hardware emulation.

***************************************************************************/

static void tail2nos_postload(running_machine &machine)
{
	tail2nos_state *state = machine.driver_data<tail2nos_state>();
	int i;

	state->m_bg_tilemap->mark_all_dirty();

	for (i = 0; i < 0x20000; i += 64)
	{
		gfx_element_mark_dirty(machine.gfx[2], i / 64);
	}
}

VIDEO_START( tail2nos )
{
	tail2nos_state *state = machine.driver_data<tail2nos_state>();

	state->m_bg_tilemap = tilemap_create(machine, get_tile_info, tilemap_scan_rows, 8, 8, 64, 32);

	state->m_bg_tilemap->set_transparent_pen(15);

	state->m_zoomdata = (UINT16 *)machine.region("gfx3")->base();

	state->save_pointer(NAME(state->m_zoomdata), 0x20000 / 2);
	machine.save().register_postload(save_prepost_delegate(FUNC(tail2nos_postload), &machine));
}



/***************************************************************************

  Memory handlers

***************************************************************************/

WRITE16_MEMBER(tail2nos_state::tail2nos_bgvideoram_w)
{

	COMBINE_DATA(&m_bgvideoram[offset]);
	m_bg_tilemap->mark_tile_dirty(offset);
}

READ16_MEMBER(tail2nos_state::tail2nos_zoomdata_r)
{
	return m_zoomdata[offset];
}

WRITE16_MEMBER(tail2nos_state::tail2nos_zoomdata_w)
{
	int oldword = m_zoomdata[offset];

	COMBINE_DATA(&m_zoomdata[offset]);
	if (oldword != m_zoomdata[offset])
		gfx_element_mark_dirty(machine().gfx[2], offset / 64);
}

WRITE16_MEMBER(tail2nos_state::tail2nos_gfxbank_w)
{

	if (ACCESSING_BITS_0_7)
	{
		int bank;

		/* bits 0 and 2 select char bank */
		if (data & 0x04)
			bank = 2;
		else if (data & 0x01)
			bank = 1;
		else
			bank = 0;

		if (m_charbank != bank)
		{
			m_charbank = bank;
			m_bg_tilemap->mark_all_dirty();
		}

		/* bit 5 seems to select palette bank (used on startup) */
		if (data & 0x20)
			bank = 7;
		else
			bank = 3;

		if (m_charpalette != bank)
		{
			m_charpalette = bank;
			m_bg_tilemap->mark_all_dirty();
		}

		/* bit 4 seems to be video enable */
		m_video_enable = data & 0x10;
	}
}


/***************************************************************************

    Display Refresh

***************************************************************************/

static void draw_sprites( running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	tail2nos_state *state = machine.driver_data<tail2nos_state>();
	UINT16 *spriteram = state->m_spriteram;
	int offs;


	for (offs = 0; offs < state->m_spriteram_size / 2; offs += 4)
	{
		int sx, sy, flipx, flipy, code, color;

		sx = spriteram[offs + 1];
		if (sx >= 0x8000)
			sx -= 0x10000;
		sy = 0x10000 - spriteram[offs + 0];
		if (sy >= 0x8000)
			sy -= 0x10000;
		code = spriteram[offs + 2] & 0x07ff;
		color = (spriteram[offs + 2] & 0xe000) >> 13;
		flipx = spriteram[offs + 2] & 0x1000;
		flipy = spriteram[offs + 2] & 0x0800;

		drawgfx_transpen(bitmap,/* placement relative to zoom layer verified on the real thing */
				cliprect,machine.gfx[1],
				code,
				40 + color,
				flipx,flipy,
				sx+3,sy+1,15);
	}
}

SCREEN_UPDATE_IND16( tail2nos )
{
	tail2nos_state *state = screen.machine().driver_data<tail2nos_state>();

	if (state->m_video_enable)
	{
		k051316_zoom_draw(state->m_k051316, bitmap, cliprect, 0, 0);
		draw_sprites(screen.machine(), bitmap, cliprect);
		state->m_bg_tilemap->draw(bitmap, cliprect, 0, 0);
	}
	else
		bitmap.fill(0, cliprect);

	return 0;
}
