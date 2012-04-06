/***************************************************************************

  video.c

  Functions to emulate the video hardware of the machine.

***************************************************************************/
#include "emu.h"
#include "includes/nycaptor.h"

#define NYCAPTOR_DEBUG	0

/*
 298 (e298) - spot (0-3) , 299 (e299) - lives
 spot number isn't set to 0 in main menu ; lives - yes
 sprites in main menu req priority 'type' 0
*/
static int nycaptor_spot( running_machine &machine )
{
	nycaptor_state *state = machine.driver_data<nycaptor_state>();

	if (state->m_gametype == 0 || state->m_gametype == 2)
		return state->m_sharedram[0x299] ? state->m_sharedram[0x298] : 0;
	else
		return 0;
}

WRITE8_MEMBER(nycaptor_state::nycaptor_spriteram_w)
{
	m_spriteram[offset] = data;
}

READ8_MEMBER(nycaptor_state::nycaptor_spriteram_r)
{
	return m_spriteram[offset];
}

static TILE_GET_INFO( get_tile_info )
{
	nycaptor_state *state = machine.driver_data<nycaptor_state>();
	int pal = state->m_videoram[tile_index * 2 + 1] & 0x0f;
	tileinfo.category = (state->m_videoram[tile_index * 2 + 1] & 0x30) >> 4;

	tileinfo.group = 0;

	if ((!nycaptor_spot(machine)) && (pal == 6))
		tileinfo.group = 1;

	if (((nycaptor_spot(machine) == 3) && (pal == 8)) || ((nycaptor_spot(machine) == 1) && (pal == 0xc)))
		tileinfo.group = 2;

	if ((nycaptor_spot(machine) == 1) && (tileinfo.category == 2))
		tileinfo.group = 3;

#if NYCAPTOR_DEBUG
	if (state->m_mask & (1 << tileinfo.category))
	{
		if (nycaptor_spot(machine))
			pal = 0xe;
		else
			pal = 4;
	}
#endif

	SET_TILE_INFO(
			0,
			state->m_videoram[tile_index * 2] + ((state->m_videoram[tile_index * 2 + 1] & 0xc0) << 2) + 0x400 * state->m_char_bank,
			pal, 0
			);
}


VIDEO_START( nycaptor )
{
	nycaptor_state *state = machine.driver_data<nycaptor_state>();

	state->m_spriteram = auto_alloc_array(machine, UINT8, 160);
	state->m_bg_tilemap = tilemap_create(machine, get_tile_info, tilemap_scan_rows, 8, 8, 32, 32 );

	state->m_bg_tilemap->set_transmask(0, 0xf800, 0x7ff); //split 0
	state->m_bg_tilemap->set_transmask(1, 0xfe00, 0x01ff);//split 1
	state->m_bg_tilemap->set_transmask(2, 0xfffc, 0x0003);//split 2
	state->m_bg_tilemap->set_transmask(3, 0xfff0, 0x000f);//split 3

	state->m_generic_paletteram_8.allocate(0x200);
	state->m_generic_paletteram2_8.allocate(0x200);
	state->m_bg_tilemap->set_scroll_cols(32);

	state->save_pointer(NAME(state->m_spriteram), 160);
}

WRITE8_MEMBER(nycaptor_state::nycaptor_videoram_w)
{
	m_videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset >> 1);
}

READ8_MEMBER(nycaptor_state::nycaptor_videoram_r)
{
	return m_videoram[offset];
}

WRITE8_MEMBER(nycaptor_state::nycaptor_palette_w)
{

	if (m_gametype == 2) //colt
		return;

	if (offset & 0x100)
		paletteram_xxxxBBBBGGGGRRRR_split2_w(space, (offset & 0xff) + (m_palette_bank << 8), data);
	else
		paletteram_xxxxBBBBGGGGRRRR_split1_w(space, (offset & 0xff) + (m_palette_bank << 8), data);
}

READ8_MEMBER(nycaptor_state::nycaptor_palette_r)
{

	if (offset & 0x100)
		return m_generic_paletteram2_8[(offset & 0xff) + (m_palette_bank << 8)];
	else
		return m_generic_paletteram_8 [(offset & 0xff) + (m_palette_bank << 8)];
}

WRITE8_MEMBER(nycaptor_state::nycaptor_gfxctrl_w)
{

	if (m_gfxctrl == data)
		return;

	m_gfxctrl = data;

	if (m_char_bank != ((data & 0x18) >> 3))
	{
		m_char_bank = ((data & 0x18) >> 3);
		m_bg_tilemap->mark_all_dirty();
	}

	m_palette_bank = BIT(data, 5);

}

READ8_MEMBER(nycaptor_state::nycaptor_gfxctrl_r)
{
	return m_gfxctrl;
}

READ8_MEMBER(nycaptor_state::nycaptor_scrlram_r)
{
	return m_scrlram[offset];
}

WRITE8_MEMBER(nycaptor_state::nycaptor_scrlram_w)
{
	m_scrlram[offset] = data;
	m_bg_tilemap->set_scrolly(offset, data);
}

static void draw_sprites( running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect, int pri )
{
	nycaptor_state *state = machine.driver_data<nycaptor_state>();
	int i;

	for (i = 0; i < 0x20; i++)
	{
		int pr = state->m_spriteram[0x9f - i];
		int offs = (pr & 0x1f) * 4;
		int code, sx, sy, flipx, flipy, pal, priori;

		code = state->m_spriteram[offs + 2] + ((state->m_spriteram[offs + 1] & 0x10) << 4);//1 bit wolny = 0x20
		pal  = state->m_spriteram[offs + 1] & 0x0f;
		sx   = state->m_spriteram[offs + 3];
		sy   = 240 - state->m_spriteram[offs + 0];
		priori = (pr & 0xe0) >> 5;

		if (priori == pri)
		{
#if NYCAPTOR_DEBUG
			if (state->m_mask & (1 << (pri + 4))) pal = 0xd;
#endif
			flipx = BIT(state->m_spriteram[offs + 1], 6);
			flipy = BIT(state->m_spriteram[offs + 1], 7);

			drawgfx_transpen(bitmap, cliprect, machine.gfx[1],
					code,
					pal,
					flipx,flipy,
					sx,sy,15);

			if (state->m_spriteram[offs + 3] > 240)
			{
				sx = (state->m_spriteram[offs + 3] - 256);
				drawgfx_transpen(bitmap, cliprect, machine.gfx[1],
					code,
					pal,
					flipx,flipy,
					sx,sy,15);
			}
		}
	}
}





#if NYCAPTOR_DEBUG
/*
 Keys :
   q/w/e/r - bg priority display select
   a/s/d/f/g/h/j/k - sprite priority display select
   z - clear
   x - no bg/sprite pri.
*/

#define mKEY_MASK(x,y) if (machine.input().code_pressed_once(x)) { state->m_mask |= y; state->m_bg_tilemap->mark_all_dirty(); }

static void nycaptor_setmask( running_machine &machine )
{
	nycaptor_state *state = machine.driver_data<nycaptor_state>();

	mKEY_MASK(KEYCODE_Q, 1); /* bg */
	mKEY_MASK(KEYCODE_W, 2);
	mKEY_MASK(KEYCODE_E, 4);
	mKEY_MASK(KEYCODE_R, 8);

	mKEY_MASK(KEYCODE_A, 0x10); /* sprites */
	mKEY_MASK(KEYCODE_S, 0x20);
	mKEY_MASK(KEYCODE_D, 0x40);
	mKEY_MASK(KEYCODE_F, 0x80);
	mKEY_MASK(KEYCODE_G, 0x100);
	mKEY_MASK(KEYCODE_H, 0x200);
	mKEY_MASK(KEYCODE_J, 0x400);
	mKEY_MASK(KEYCODE_K, 0x800);

	if (machine.input().code_pressed_once(KEYCODE_Z)){state->m_mask = 0; state->m_bg_tilemap->mark_all_dirty();} /* disable */
	if (machine.input().code_pressed_once(KEYCODE_X)){state->m_mask |= 0x1000; state->m_bg_tilemap->mark_all_dirty();} /* no layers */
}
#endif

SCREEN_UPDATE_IND16( nycaptor )
{
	nycaptor_state *state = screen.machine().driver_data<nycaptor_state>();

#if NYCAPTOR_DEBUG
	nycaptor_setmask(screen.machine());
	if (state->m_mask & 0x1000)
	{
		state->m_bg_tilemap->draw(bitmap, cliprect, TILEMAP_DRAW_LAYER1 | 3, 0);
		state->m_bg_tilemap->draw(bitmap, cliprect, TILEMAP_DRAW_LAYER0 | 3, 0);
		state->m_bg_tilemap->draw(bitmap, cliprect, TILEMAP_DRAW_LAYER1 | 2, 0);
		state->m_bg_tilemap->draw(bitmap, cliprect, TILEMAP_DRAW_LAYER0 | 2, 0);
		state->m_bg_tilemap->draw(bitmap, cliprect, TILEMAP_DRAW_LAYER1 | 1, 0);
		state->m_bg_tilemap->draw(bitmap, cliprect, TILEMAP_DRAW_LAYER0 | 1, 0);
		state->m_bg_tilemap->draw(bitmap, cliprect, TILEMAP_DRAW_LAYER1 | 0, 0);
		state->m_bg_tilemap->draw(bitmap, cliprect, TILEMAP_DRAW_LAYER0 | 0, 0);
		draw_sprites(screen.machine(), bitmap, cliprect, 0);
		draw_sprites(screen.machine(), bitmap, cliprect, 1);
		draw_sprites(screen.machine(), bitmap, cliprect, 2);
		draw_sprites(screen.machine(), bitmap, cliprect, 3);
		draw_sprites(screen.machine(), bitmap, cliprect, 4);
		draw_sprites(screen.machine(), bitmap, cliprect, 5);
		draw_sprites(screen.machine(), bitmap, cliprect, 6);
		draw_sprites(screen.machine(), bitmap, cliprect, 7);
	}
	else
	#endif
	switch (nycaptor_spot(screen.machine()) & 3)
	{
	case 0:
		state->m_bg_tilemap->draw(bitmap, cliprect, TILEMAP_DRAW_LAYER1 | 3, 0);
		draw_sprites(screen.machine(), bitmap, cliprect, 6);
		state->m_bg_tilemap->draw(bitmap, cliprect, TILEMAP_DRAW_LAYER0 | 3, 0);
		state->m_bg_tilemap->draw(bitmap, cliprect, TILEMAP_DRAW_LAYER1 | 2, 0);
		state->m_bg_tilemap->draw(bitmap, cliprect, TILEMAP_DRAW_LAYER0 | 2, 0);
		state->m_bg_tilemap->draw(bitmap, cliprect, TILEMAP_DRAW_LAYER1 | 1, 0);
		draw_sprites(screen.machine(), bitmap, cliprect, 3);
		state->m_bg_tilemap->draw(bitmap, cliprect, TILEMAP_DRAW_LAYER0 | 1, 0);
		draw_sprites(screen.machine(), bitmap, cliprect, 0);
		draw_sprites(screen.machine(), bitmap, cliprect, 2);
		state->m_bg_tilemap->draw(bitmap, cliprect, TILEMAP_DRAW_LAYER1 | 0, 0);
		draw_sprites(screen.machine(), bitmap, cliprect, 1);
		state->m_bg_tilemap->draw(bitmap, cliprect, TILEMAP_DRAW_LAYER0 | 0, 0);
		break;

	case 1:
		state->m_bg_tilemap->draw(bitmap, cliprect, TILEMAP_DRAW_LAYER1 | 3, 0);
		draw_sprites(screen.machine(), bitmap, cliprect, 3);
		state->m_bg_tilemap->draw(bitmap, cliprect, TILEMAP_DRAW_LAYER0 | 3, 0);
		draw_sprites(screen.machine(), bitmap, cliprect, 2);
		state->m_bg_tilemap->draw(bitmap, cliprect, TILEMAP_DRAW_LAYER1 | 2, 0);
		state->m_bg_tilemap->draw(bitmap, cliprect, TILEMAP_DRAW_LAYER1 | 1, 0);
		draw_sprites(screen.machine(), bitmap, cliprect, 1);
		state->m_bg_tilemap->draw(bitmap, cliprect, TILEMAP_DRAW_LAYER0 | 1, 0);
		state->m_bg_tilemap->draw(bitmap, cliprect, TILEMAP_DRAW_LAYER0 | 2, 0);
		draw_sprites(screen.machine(), bitmap, cliprect, 0);
		state->m_bg_tilemap->draw(bitmap, cliprect, TILEMAP_DRAW_LAYER1 | 0, 0);
		state->m_bg_tilemap->draw(bitmap, cliprect, TILEMAP_DRAW_LAYER0 | 0, 0);
		break;

	case 2:
		state->m_bg_tilemap->draw(bitmap, cliprect, TILEMAP_DRAW_LAYER1 | 3, 0);
		state->m_bg_tilemap->draw(bitmap, cliprect, TILEMAP_DRAW_LAYER0 | 3, 0);
		state->m_bg_tilemap->draw(bitmap, cliprect, TILEMAP_DRAW_LAYER1 | 1, 0);
		draw_sprites(screen.machine(), bitmap, cliprect, 1);
		state->m_bg_tilemap->draw(bitmap, cliprect, TILEMAP_DRAW_LAYER0 | 1, 0);
		state->m_bg_tilemap->draw(bitmap, cliprect, TILEMAP_DRAW_LAYER1 | 2, 0);
		state->m_bg_tilemap->draw(bitmap, cliprect, TILEMAP_DRAW_LAYER0 | 2, 0);
		draw_sprites(screen.machine(), bitmap, cliprect, 0);
		state->m_bg_tilemap->draw(bitmap, cliprect, TILEMAP_DRAW_LAYER1 | 0, 0);
		state->m_bg_tilemap->draw(bitmap, cliprect, TILEMAP_DRAW_LAYER0 | 0, 0);
		break;

	case 3:
		state->m_bg_tilemap->draw(bitmap, cliprect, TILEMAP_DRAW_LAYER1 | 1, 0);
		draw_sprites(screen.machine(), bitmap, cliprect, 1);
		state->m_bg_tilemap->draw(bitmap, cliprect, TILEMAP_DRAW_LAYER0 | 1, 0);
		draw_sprites(screen.machine(), bitmap, cliprect, 0);
		state->m_bg_tilemap->draw(bitmap, cliprect, TILEMAP_DRAW_LAYER1 | 0, 0);
		state->m_bg_tilemap->draw(bitmap, cliprect, TILEMAP_DRAW_LAYER0 | 0, 0);
		break;
	}

	return 0;
}

