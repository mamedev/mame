// license:BSD-3-Clause
// copyright-holders:Luca Elia
/**************************************************************************

                            Ginga NinkyouDen
                            (C) 1987 Jaleco

                    driver by Luca Elia (l.elia@tin.it)


Note:   if MAME_DEBUG is defined, pressing Z with:

        Q       shows background
        W       shows foreground
        E       shows frontmost (text) layer
        A       shows sprites

        Keys can be used togheter!


[Screen]
    Visible Size:       256H x 240V
    Dynamic Colors:     256 x 4
    Color Space:        16R x 16G x 16B

[Scrolling layers]
    Format (all layers):    Offset:     0x400    0x000
                            Bit:        fedc---- --------   Color
                                        ----ba98 76543210   Code

    [Background]
        Size:               8192 x 512  (static: stored in ROM)
        Scrolling:          X,Y         (registers: $60006.w, $60004.w)
        Tiles Size:         16 x 16
        Tiles Number:       $400
        Colors:             $300-$3ff

    [Foreground]
        Size:               4096 x 512
        Scrolling:          X,Y         (registers: $60002.w, $60000.w)
        Tiles Size:         16 x 16
        Tiles Number:       $400
        Colors:             $200-$2ff

    [Frontmost]
        Size:               256 x 256
        Scrolling:          -
        Tiles Size:         8 x 8
        Tiles Number:       $200
        Colors:             $000-$0ff


[Sprites]
    On Screen:          256
    In ROM:             $a00
    Colors:             $100-$1ff
    Format:             See Below


**************************************************************************/

#include "emu.h"
#include "includes/ginganin.h"


/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/


/* Background - Resides in ROM */

#define BG_GFX (0)
#define BG_NX  (16*32)
#define BG_NY  (16*2)

TILE_GET_INFO_MEMBER(ginganin_state::get_bg_tile_info)
{
	UINT8 *gfx = memregion("gfx5")->base();
	int code = gfx[2 * tile_index + 0] * 256 + gfx[2 * tile_index + 1];
	SET_TILE_INFO_MEMBER(BG_GFX,
			code,
			code >> 12,
			0);
}


/* Foreground - Resides in RAM */

#define FG_GFX (1)
#define FG_NX  (16*16)
#define FG_NY  (16*2)

TILE_GET_INFO_MEMBER(ginganin_state::get_fg_tile_info)
{
	UINT16 code = m_fgram[tile_index];
	SET_TILE_INFO_MEMBER(FG_GFX,
			code,
			code >> 12,
			0);
}

WRITE16_MEMBER(ginganin_state::ginganin_fgram16_w)
{
	COMBINE_DATA(&m_fgram[offset]);
	m_fg_tilemap->mark_tile_dirty(offset);
}


/* Frontmost (text) Layer - Resides in RAM */

#define TXT_GFX (2)
#define TXT_NX  (32)
#define TXT_NY  (32)

TILE_GET_INFO_MEMBER(ginganin_state::get_txt_tile_info)
{
	UINT16 code = m_txtram[tile_index];
	SET_TILE_INFO_MEMBER(TXT_GFX,
			code,
			code >> 12,
			0);
}

WRITE16_MEMBER(ginganin_state::ginganin_txtram16_w)
{
	COMBINE_DATA(&m_txtram[offset]);
	m_tx_tilemap->mark_tile_dirty(offset);
}


void ginganin_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(ginganin_state::get_bg_tile_info),this), TILEMAP_SCAN_COLS, 16, 16, BG_NX, BG_NY);
	m_fg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(ginganin_state::get_fg_tile_info),this), TILEMAP_SCAN_COLS, 16, 16, FG_NX, FG_NY);
	m_tx_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(ginganin_state::get_txt_tile_info),this), TILEMAP_SCAN_ROWS, 8, 8, TXT_NX, TXT_NY);

	m_fg_tilemap->set_transparent_pen(15);
	m_tx_tilemap->set_transparent_pen(15);
}


WRITE16_MEMBER(ginganin_state::ginganin_vregs16_w)
{
	COMBINE_DATA(&m_vregs[offset]);
	data = m_vregs[offset];

	switch (offset)
	{
	case 0:
		m_fg_tilemap->set_scrolly(0, data);
		break;
	case 1:
		m_fg_tilemap->set_scrollx(0, data);
		break;
	case 2:
		m_bg_tilemap->set_scrolly(0, data);
		break;
	case 3:
		m_bg_tilemap->set_scrollx(0, data);
		break;
	case 4:
		m_layers_ctrl = data;
		break;
/*  case 5:
 *      break;
 */
	case 6:
		m_flipscreen = !(data & 1);
		machine().tilemap().set_flip_all(m_flipscreen ? (TILEMAP_FLIPY | TILEMAP_FLIPX) : 0);
		break;
	case 7:
		soundlatch_byte_w(space, 0, data);
		m_audiocpu->set_input_line(INPUT_LINE_NMI, PULSE_LINE);
		break;
	default:
		logerror("CPU #0 PC %06X : Warning, videoreg %04X <- %04X\n", space.device().safe_pc(), offset, data);
	}
}



/* --------------------------[ Sprites Format ]----------------------------

Offset:         Values:         Format:

0000.w          y position      fedc ba9- ---- ----     unused
                                ---- ---8 ---- ----     subtract 256
                                ---- ---- 7654 3210     position

0002.w          x position      See above

0004.w          code            f--- ---- ---- ----     y flip
                                -e-- ---- ---- ----     x flip
                                --dc ---- ---- ----     unused?
                                ---- ba98 7654 3210     code

0006.w          colour          fedc ---- ---- ----     colour code
                                ---- ba98 7654 3210     unused?

------------------------------------------------------------------------ */

void ginganin_state::draw_sprites( bitmap_ind16 &bitmap,const rectangle &cliprect )
{
	UINT16 *spriteram = m_spriteram;
	int offs;

	for (offs = 0; offs < (m_spriteram.bytes() >> 1); offs += 4)
	{
		int y = spriteram[offs + 0];
		int x = spriteram[offs + 1];
		int code = spriteram[offs + 2];
		int attr = spriteram[offs + 3];
		int flipx = code & 0x4000;
		int flipy = code & 0x8000;

		x = (x & 0xff) - (x & 0x100);
		y = (y & 0xff) - (y & 0x100);

		if (m_flipscreen)
		{
			x = 240 - x;
			y = 240 - y;
			flipx = !flipx;
			flipy = !flipy;
		}

		m_gfxdecode->gfx(3)->transpen(bitmap,cliprect,
				code & 0x3fff,
				attr >> 12,
				flipx, flipy,
				x,y,15);

	}
}


UINT32 ginganin_state::screen_update_ginganin(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int layers_ctrl1 = m_layers_ctrl;

#ifdef MAME_DEBUG
if (machine().input().code_pressed(KEYCODE_Z))
{
	int msk = 0;

	if (machine().input().code_pressed(KEYCODE_Q)) { msk |= 0xfff1;}
	if (machine().input().code_pressed(KEYCODE_W)) { msk |= 0xfff2;}
	if (machine().input().code_pressed(KEYCODE_E)) { msk |= 0xfff4;}
	if (machine().input().code_pressed(KEYCODE_A))  { msk |= 0xfff8;}
	if (msk != 0) layers_ctrl1 &= msk;

#define SETSCROLL \
	m_bg_tilemap->set_scrollx(0, m_posx); \
	m_bg_tilemap->set_scrolly(0, m_posy); \
	m_fg_tilemap->set_scrollx(0, m_posx); \
	m_fg_tilemap->set_scrolly(0, m_posy); \
	popmessage("B>%04X:%04X F>%04X:%04X",m_posx%(BG_NX*16),m_posy%(BG_NY*16),m_posx%(FG_NX*16),m_posy%(FG_NY*16));

	if (machine().input().code_pressed(KEYCODE_L))  { m_posx +=8; SETSCROLL }
	if (machine().input().code_pressed(KEYCODE_J))  { m_posx -=8; SETSCROLL }
	if (machine().input().code_pressed(KEYCODE_K))  { m_posy +=8; SETSCROLL }
	if (machine().input().code_pressed(KEYCODE_I))  { m_posy -=8; SETSCROLL }
	if (machine().input().code_pressed(KEYCODE_H))  { m_posx = m_posy = 0;  SETSCROLL }

}
#endif


	if (layers_ctrl1 & 1)
		m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	else
		bitmap.fill(0, cliprect);

	if (layers_ctrl1 & 2)
		m_fg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	if (layers_ctrl1 & 8)
		draw_sprites(bitmap, cliprect);
	if (layers_ctrl1 & 4)
		m_tx_tilemap->draw(screen, bitmap, cliprect, 0, 0);

	return 0;
}
