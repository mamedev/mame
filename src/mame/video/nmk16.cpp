// license:BSD-3-Clause
// copyright-holders:Mirko Buffoni,Nicola Salmoria,Bryan McPhail,David Haywood,R. Belmont,Alex Marshall,Angelo Salese,Luca Elia
// thanks-to:Richard Bush
/* notes...

 drawing sprites in a single pass with pdrawgfx breaks Thunder Dragon 2,
  which seems to expect the sprite priority values to affect sprite-sprite
  priority.  Thunder Dragon 2 also breaks if you support sprite flipping,
  the collectible point score / power up names appear flipped..

*/

#include "emu.h"
#include "includes/nmk16.h"

// the larger tilemaps on macross2, rapid hero and thunder dragon 2 appear to act like 4 'banks'
// of the smaller tilemaps, rather than being able to scroll into each other (not verified on real hw,
// but see raphero intro / 1st level cases)


/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

#define TILES_PER_PAGE_X    (0x10)
#define TILES_PER_PAGE_Y    (0x10)
#define PAGES_PER_TMAP_X    (0x10)
#define PAGES_PER_TMAP_Y    (0x02)

TILEMAP_MAPPER_MEMBER(nmk16_state::afega_tilemap_scan_pages)
{
	return  (row / TILES_PER_PAGE_Y) * TILES_PER_PAGE_X * TILES_PER_PAGE_Y * PAGES_PER_TMAP_X +
			(row % TILES_PER_PAGE_Y) +

			(col / TILES_PER_PAGE_X) * TILES_PER_PAGE_X * TILES_PER_PAGE_Y +
			(col % TILES_PER_PAGE_X) * TILES_PER_PAGE_Y;
}

TILE_GET_INFO_MEMBER(nmk16_state::macross_get_bg0_tile_info)
{
	int code = m_nmk_bgvideoram0[tile_index];
	SET_TILE_INFO_MEMBER(1,(code & 0xfff) + (m_bgbank << 12),code >> 12,0);
}

TILE_GET_INFO_MEMBER(nmk16_state::macross_get_bg1_tile_info)
{
	int code = m_nmk_bgvideoram1[tile_index];
	SET_TILE_INFO_MEMBER(1,(code & 0xfff) + (m_bgbank << 12),code >> 12,0);
}

TILE_GET_INFO_MEMBER(nmk16_state::macross_get_bg2_tile_info)
{
	int code = m_nmk_bgvideoram2[tile_index];
	SET_TILE_INFO_MEMBER(1,(code & 0xfff) + (m_bgbank << 12),code >> 12,0);
}

TILE_GET_INFO_MEMBER(nmk16_state::macross_get_bg3_tile_info)
{
	int code = m_nmk_bgvideoram3[tile_index];
	SET_TILE_INFO_MEMBER(1,(code & 0xfff) + (m_bgbank << 12),code >> 12,0);
}


TILE_GET_INFO_MEMBER(nmk16_state::strahl_get_fg_tile_info)
{
	int code = m_nmk_fgvideoram[tile_index];
	SET_TILE_INFO_MEMBER(3,
			(code & 0xfff),
			code >> 12,
			0);
}

TILE_GET_INFO_MEMBER(nmk16_state::macross_get_tx_tile_info)
{
	int code = m_nmk_txvideoram[tile_index];
	SET_TILE_INFO_MEMBER(0,
			code & 0xfff,
			code >> 12,
			0);
}

TILE_GET_INFO_MEMBER(nmk16_state::bjtwin_get_bg_tile_info)
{
	int code = m_nmk_bgvideoram0[tile_index];
	int bank = (code & 0x800) ? 1 : 0;
	SET_TILE_INFO_MEMBER(bank,
			(code & 0x7ff) + ((bank) ? (m_bgbank << 11) : 0),
			code >> 12,
			0);
}

TILE_GET_INFO_MEMBER(nmk16_state::get_tile_info_0_8bit)
{
	UINT16 code = m_nmk_bgvideoram0[tile_index];
	SET_TILE_INFO_MEMBER(1,
			code,
			0,
			0);
}


/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

void nmk16_state::nmk16_video_init()
{
	m_spriteram_old = auto_alloc_array_clear(machine(), UINT16, 0x1000/2);
	m_spriteram_old2 = auto_alloc_array_clear(machine(), UINT16, 0x1000/2);

	m_videoshift = 0;        /* 256x224 screen, no shift */
	m_background_bitmap = nullptr;
	m_simple_scroll = 1;
}


VIDEO_START_MEMBER(nmk16_state,bioship)
{
	m_bg_tilemap0 = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(nmk16_state::macross_get_bg0_tile_info),this), tilemap_mapper_delegate(FUNC(nmk16_state::afega_tilemap_scan_pages),this),16,16,TILES_PER_PAGE_X*PAGES_PER_TMAP_X,TILES_PER_PAGE_Y*PAGES_PER_TMAP_Y);
	m_tx_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(nmk16_state::macross_get_tx_tile_info),this),TILEMAP_SCAN_COLS,8,8,32,32);

	m_bg_tilemap0->set_transparent_pen(15);
	m_tx_tilemap->set_transparent_pen(15);

	nmk16_video_init();
	m_background_bitmap = auto_bitmap_ind16_alloc(machine(),8192,512);
	m_bioship_background_bank=0;
	m_redraw_bitmap = 1;

}

VIDEO_START_MEMBER(nmk16_state,strahl)
{
	m_bg_tilemap0 = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(nmk16_state::macross_get_bg0_tile_info),this), tilemap_mapper_delegate(FUNC(nmk16_state::afega_tilemap_scan_pages),this),16,16,TILES_PER_PAGE_X*PAGES_PER_TMAP_X,TILES_PER_PAGE_Y*PAGES_PER_TMAP_Y);
	m_fg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(nmk16_state::strahl_get_fg_tile_info),this), tilemap_mapper_delegate(FUNC(nmk16_state::afega_tilemap_scan_pages),this),16,16,TILES_PER_PAGE_X*PAGES_PER_TMAP_X,TILES_PER_PAGE_Y*PAGES_PER_TMAP_Y);
	m_tx_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(nmk16_state::macross_get_tx_tile_info),this),TILEMAP_SCAN_COLS,8,8,32,32);

	m_fg_tilemap->set_transparent_pen(15);
	m_tx_tilemap->set_transparent_pen(15);

	m_sprdma_base = 0xf000;

	nmk16_video_init();
}

VIDEO_START_MEMBER(nmk16_state,macross)
{
	m_bg_tilemap0 = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(nmk16_state::macross_get_bg0_tile_info),this), tilemap_mapper_delegate(FUNC(nmk16_state::afega_tilemap_scan_pages),this),16,16,TILES_PER_PAGE_X*PAGES_PER_TMAP_X,TILES_PER_PAGE_Y*PAGES_PER_TMAP_Y);
	m_tx_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(nmk16_state::macross_get_tx_tile_info),this),TILEMAP_SCAN_COLS,8,8,32,32);

	m_tx_tilemap->set_transparent_pen(15);

	nmk16_video_init();
}

VIDEO_START_MEMBER(nmk16_state,gunnail)
{
	m_bg_tilemap0 = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(nmk16_state::macross_get_bg0_tile_info),this), tilemap_mapper_delegate(FUNC(nmk16_state::afega_tilemap_scan_pages),this),16,16,TILES_PER_PAGE_X*PAGES_PER_TMAP_X,TILES_PER_PAGE_Y*PAGES_PER_TMAP_Y);
	m_tx_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(nmk16_state::macross_get_tx_tile_info),this),TILEMAP_SCAN_COLS,8,8,64,32);

	m_tx_tilemap->set_transparent_pen(15);
	m_bg_tilemap0->set_scroll_rows(512);

	nmk16_video_init();
	m_videoshift = 64;  /* 384x224 screen, leftmost 64 pixels have to be retrieved */
						/* from the other side of the tilemap (!) */
	m_simple_scroll = 0;
}

VIDEO_START_MEMBER(nmk16_state,macross2)
{
	m_bg_tilemap0 = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(nmk16_state::macross_get_bg0_tile_info),this), tilemap_mapper_delegate(FUNC(nmk16_state::afega_tilemap_scan_pages),this),16,16,TILES_PER_PAGE_X*PAGES_PER_TMAP_X,TILES_PER_PAGE_Y*PAGES_PER_TMAP_Y);
	m_bg_tilemap1 = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(nmk16_state::macross_get_bg1_tile_info),this), tilemap_mapper_delegate(FUNC(nmk16_state::afega_tilemap_scan_pages),this),16,16,TILES_PER_PAGE_X*PAGES_PER_TMAP_X,TILES_PER_PAGE_Y*PAGES_PER_TMAP_Y);
	m_bg_tilemap2 = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(nmk16_state::macross_get_bg2_tile_info),this), tilemap_mapper_delegate(FUNC(nmk16_state::afega_tilemap_scan_pages),this),16,16,TILES_PER_PAGE_X*PAGES_PER_TMAP_X,TILES_PER_PAGE_Y*PAGES_PER_TMAP_Y);
	m_bg_tilemap3 = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(nmk16_state::macross_get_bg3_tile_info),this), tilemap_mapper_delegate(FUNC(nmk16_state::afega_tilemap_scan_pages),this),16,16,TILES_PER_PAGE_X*PAGES_PER_TMAP_X,TILES_PER_PAGE_Y*PAGES_PER_TMAP_Y);

	m_tx_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(nmk16_state::macross_get_tx_tile_info),this),TILEMAP_SCAN_COLS,8,8,64,32);

	m_tx_tilemap->set_transparent_pen(15);

	nmk16_video_init();
	m_videoshift = 64;  /* 384x224 screen, leftmost 64 pixels have to be retrieved */
						/* from the other side of the tilemap (!) */
}

VIDEO_START_MEMBER(nmk16_state,raphero)
{
	VIDEO_START_CALL_MEMBER( macross2 );
	m_simple_scroll = 0;
}

VIDEO_START_MEMBER(nmk16_state,bjtwin)
{
	m_bg_tilemap0 = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(nmk16_state::bjtwin_get_bg_tile_info),this),TILEMAP_SCAN_COLS,8,8,64,32);

	nmk16_video_init();
	m_videoshift = 64;  /* 384x224 screen, leftmost 64 pixels have to be retrieved */
						/* from the other side of the tilemap (!) */
}



/***************************************************************************

  Memory handlers

***************************************************************************/

WRITE16_MEMBER(nmk16_state::nmk_bgvideoram0_w)
{
	COMBINE_DATA(&m_nmk_bgvideoram0[offset]);
	m_bg_tilemap0->mark_tile_dirty(offset);
}

WRITE16_MEMBER(nmk16_state::nmk_bgvideoram1_w)
{
	COMBINE_DATA(&m_nmk_bgvideoram1[offset]);
	m_bg_tilemap1->mark_tile_dirty(offset);
}

WRITE16_MEMBER(nmk16_state::nmk_bgvideoram2_w)
{
	COMBINE_DATA(&m_nmk_bgvideoram2[offset]);
	m_bg_tilemap2->mark_tile_dirty(offset);
}

WRITE16_MEMBER(nmk16_state::nmk_bgvideoram3_w)
{
	COMBINE_DATA(&m_nmk_bgvideoram3[offset]);
	m_bg_tilemap3->mark_tile_dirty(offset);
}

WRITE16_MEMBER(nmk16_state::nmk_fgvideoram_w)
{
	COMBINE_DATA(&m_nmk_fgvideoram[offset]);
	m_fg_tilemap->mark_tile_dirty(offset);
}

WRITE16_MEMBER(nmk16_state::nmk_txvideoram_w)
{
	COMBINE_DATA(&m_nmk_txvideoram[offset]);
	m_tx_tilemap->mark_tile_dirty(offset);
}


WRITE16_MEMBER(nmk16_state::mustang_scroll_w)
{
//  osd_printf_debug("mustang %04x %04x %04x\n",offset,data,mem_mask);

	switch (data & 0xff00)
	{
		case 0x0000:
			m_mustang_bg_xscroll = (m_mustang_bg_xscroll & 0x00ff) | ((data & 0x00ff)<<8);
			break;

		case 0x0100:
			m_mustang_bg_xscroll = (m_mustang_bg_xscroll & 0xff00) | (data & 0x00ff);
			break;

		case 0x0200:
			break;

		case 0x0300:
			break;

		default:
			break;
	}

	m_bg_tilemap0->set_scrollx(0,m_mustang_bg_xscroll - m_videoshift);
}

WRITE16_MEMBER(nmk16_state::bioshipbg_scroll_w)
{
	if (ACCESSING_BITS_8_15)
	{
		m_scroll[offset] = (data >> 8) & 0xff;

		if (offset & 2)
			m_bg_tilemap0->set_scrolly(0,m_scroll[2] * 256 + m_scroll[3]);
		else
			m_bg_tilemap0->set_scrollx(0,m_scroll[0] * 256 + m_scroll[1] - m_videoshift);
	}
}

WRITE16_MEMBER(nmk16_state::nmk_scroll_w)
{
	if (ACCESSING_BITS_0_7)
	{
		m_scroll[offset] = data & 0xff;

		if (offset & 2)
			m_bg_tilemap0->set_scrolly(0,m_scroll[2] * 256 + m_scroll[3]);
		else
			m_bg_tilemap0->set_scrollx(0,m_scroll[0] * 256 + m_scroll[1] - m_videoshift);
	}
}

WRITE16_MEMBER(nmk16_state::nmk_scroll_2_w)
{
	if (ACCESSING_BITS_0_7)
	{
		m_scroll_2[offset] = data & 0xff;

		if (offset & 2)
			m_fg_tilemap->set_scrolly(0,m_scroll_2[2] * 256 + m_scroll_2[3]);
		else
			m_fg_tilemap->set_scrollx(0,m_scroll_2[0] * 256 + m_scroll_2[1] - m_videoshift);
	}
}

WRITE16_MEMBER(nmk16_state::vandyke_scroll_w)
{
	m_vscroll[offset] = data;

	m_bg_tilemap0->set_scrollx(0,m_vscroll[0] * 256 + (m_vscroll[1] >> 8));
	m_bg_tilemap0->set_scrolly(0,m_vscroll[2] * 256 + (m_vscroll[3] >> 8));
}

WRITE16_MEMBER(nmk16_state::vandykeb_scroll_w)
{
	switch (offset)
	{
	case 0: COMBINE_DATA(&m_vscroll[3]); break;
	case 1: COMBINE_DATA(&m_vscroll[2]); break;
	case 5: COMBINE_DATA(&m_vscroll[1]); break;
	case 6: COMBINE_DATA(&m_vscroll[0]); break;
	}

	m_bg_tilemap0->set_scrollx(0,m_vscroll[0] * 256 + (m_vscroll[1] >> 8));
	m_bg_tilemap0->set_scrolly(0,m_vscroll[2] * 256 + (m_vscroll[3] >> 8));
}

WRITE16_MEMBER(nmk16_state::manybloc_scroll_w)
{
	COMBINE_DATA(&m_gunnail_scrollram[offset]);

	m_bg_tilemap0->set_scrollx(0,m_gunnail_scrollram[0x82/2]-m_videoshift);
	m_bg_tilemap0->set_scrolly(0,m_gunnail_scrollram[0xc2/2]);
}

WRITE16_MEMBER(nmk16_state::nmk_flipscreen_w)
{
	if (ACCESSING_BITS_0_7)
		flip_screen_set(data & 0x01);
}

WRITE16_MEMBER(nmk16_state::nmk_tilebank_w)
{
	if (ACCESSING_BITS_0_7)
	{
		if (m_bgbank != (data & 0xff))
		{
			m_bgbank = data & 0xff;
			if (m_bg_tilemap0) m_bg_tilemap0->mark_all_dirty();
			if (m_bg_tilemap1) m_bg_tilemap1->mark_all_dirty();
			if (m_bg_tilemap2) m_bg_tilemap2->mark_all_dirty();
			if (m_bg_tilemap3) m_bg_tilemap3->mark_all_dirty();
		}
	}
}

WRITE16_MEMBER(nmk16_state::bioship_scroll_w)
{
	if (ACCESSING_BITS_8_15)
		m_bioship_scroll[offset]=data>>8;
}

WRITE16_MEMBER(nmk16_state::bioship_bank_w)
{
	if (ACCESSING_BITS_0_7)
	{
		if (m_bioship_background_bank != data)
		{
			m_bioship_background_bank = data;
			m_redraw_bitmap=1;
		}
	}
}

/***************************************************************************

  Display refresh

***************************************************************************/



// manybloc uses extra flip bits on the sprites, but these break other games

inline void nmk16_state::nmk16_draw_sprite(bitmap_ind16 &bitmap, const rectangle &cliprect, UINT16 *spr)
{
	if(!(spr[0] & 0x0001))
		return;

	int sx    = (spr[4] & 0x1FF) + m_videoshift;
	int sy    =  spr[6] & 0x1FF;
	int code  =  spr[3];
	int color =  spr[7];
	int w     =  spr[1] & 0x00F;
	int h     = (spr[1] & 0x0F0) >> 4;
	int xx,yy,x;
	int delta = 16;

	if (flip_screen())
	{
		sx = 368 - sx;
		sy = 240 - sy;
		delta = -16;
	}

	yy = h;
	do
	{
		x = sx;
		xx = w;
		do
		{
		m_gfxdecode->gfx(2)->transpen(bitmap,cliprect,
			code,
			color,
			flip_screen(), flip_screen(),
			((x + 16) & 0x1FF) - 16,sy & 0x1FF,15);
		code++;
		x += delta;
		} while (--xx >= 0);

		sy += delta;
	} while (--yy >= 0);
}

inline void nmk16_state::nmk16_draw_sprite_flipsupported(bitmap_ind16 &bitmap, const rectangle &cliprect, UINT16 *spr)
{
	if(!(spr[0] & 0x0001))
		return;

	int sx    = (spr[4] & 0x1FF) + m_videoshift;
	int sy    =  spr[6] & 0x1FF;
	int code  =  spr[3];
	int color =  spr[7];
	int w     =  spr[1] & 0x00F;
	int h     = (spr[1] & 0x0F0) >> 4;
	int flipy = (spr[1] & 0x200) >> 9;
	int flipx = (spr[1] & 0x100) >> 8;

	int xx,yy,x;
	int delta = 16;

	flipx ^= flip_screen();
	flipy ^= flip_screen();

	if (flip_screen())
	{
		sx = 368 - sx;
		sy = 240 - sy;
		delta = -16;
	}

	yy = h;
	sy += flipy ? (delta*h) : 0;
	do
	{
		x = sx + (flipx ? (delta*w) : 0);
		xx = w;
		do
		{
		m_gfxdecode->gfx(2)->transpen(bitmap,cliprect,
			code,
			color,
			flipx, flipy,
			((x + 16) & 0x1FF) - 16,sy & 0x1FF,15);
		code++;
		x += delta * (flipx ? -1 : 1);
		} while (--xx >= 0);
		sy += delta * (flipy ? -1 : 1);
	} while (--yy >= 0);
}

void nmk16_state::nmk16_draw_sprites_swap(bitmap_ind16 &bitmap, const rectangle &cliprect, int *bittbl)
{
	int i;

	for (i = 0; i < 0x100; i++)
	{
		int spr = BITSWAP8(i, bittbl[0], bittbl[1], bittbl[2], bittbl[3], bittbl[4], bittbl[5], bittbl[6], bittbl[7]);
		nmk16_draw_sprite(bitmap, cliprect, m_spriteram_old2 + (spr * 16/2));
	}
}

void nmk16_state::nmk16_draw_sprites_swap_flipsupported(bitmap_ind16 &bitmap, const rectangle &cliprect, int *bittbl)
{
	int i;

	for ( i = 0; i < 0x100; i++ )
	{
		int spr = BITSWAP8(i, bittbl[0], bittbl[1], bittbl[2], bittbl[3], bittbl[4], bittbl[5], bittbl[6], bittbl[7]);
		nmk16_draw_sprite_flipsupported(bitmap, cliprect, m_spriteram_old2 + (spr * 16/2));
	}
}

void nmk16_state::nmk16_draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int offs;

	for (offs = 0; offs < 0x1000/2; offs += 8)
	{
		nmk16_draw_sprite(bitmap, cliprect, m_spriteram_old2 + offs);
	}
}

void nmk16_state::nmk16_draw_sprites_flipsupported(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int offs;

	for (offs = 0; offs < 0x1000/2; offs += 8)
	{
		nmk16_draw_sprite_flipsupported(bitmap, cliprect, m_spriteram_old2 + offs);
	}
}

/***************************************************************************


                            Generic Screen Updates


***************************************************************************/

int nmk16_state::nmk16_bg_spr_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap0->set_scrollx(0,-m_videoshift);

	m_bg_tilemap0->draw(screen, bitmap, cliprect, 0,0);

	nmk16_draw_sprites(bitmap,cliprect);
	return 0;
}

int nmk16_state::nmk16_bg_fg_spr_tx_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_tx_tilemap->set_scrollx(0,-m_videoshift);

	m_bg_tilemap0->draw(screen, bitmap, cliprect, 0,0);
	m_fg_tilemap->draw(screen, bitmap, cliprect, 0,0);

	nmk16_draw_sprites(bitmap,cliprect);

	m_tx_tilemap->draw(screen, bitmap, cliprect, 0,0);
	return 0;
}

int nmk16_state::nmk16_bg_spr_tx_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_tx_tilemap->set_scrollx(0,-m_videoshift);

	m_bg_tilemap0->draw(screen, bitmap, cliprect, 0,0);

	nmk16_draw_sprites(bitmap,cliprect);

	m_tx_tilemap->draw(screen, bitmap, cliprect, 0,0);
	return 0;
}

int nmk16_state::nmk16_bg_sprflip_tx_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_tx_tilemap->set_scrollx(0,-m_videoshift);

	m_bg_tilemap0->draw(screen, bitmap, cliprect, 0,0);

	nmk16_draw_sprites_flipsupported(bitmap,cliprect);

	m_tx_tilemap->draw(screen, bitmap, cliprect, 0,0);
	return 0;
}

int nmk16_state::nmk16_bioshipbg_sprflip_tx_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	UINT16 *tilerom = (UINT16 *)memregion("gfx5")->base();
	int scrollx=-(m_bioship_scroll[1] + m_bioship_scroll[0]*256);
	int scrolly=-(m_bioship_scroll[3] + m_bioship_scroll[2]*256);

	m_tx_tilemap->set_scrollx(0,-m_videoshift);

	if (m_redraw_bitmap)
	{
		int bank = m_bioship_background_bank * 0x2000;
		int sx=0, sy=0, offs;
		m_redraw_bitmap=0;

		/* Draw background from tile rom */
		for (offs = 0;offs <0x1000;offs++) {
				UINT16 data = tilerom[offs+bank];
				int numtile = data&0xfff;
				int color = (data&0xf000)>>12;

				m_gfxdecode->gfx(3)->opaque(*m_background_bitmap,m_background_bitmap->cliprect(),
						numtile,
						color,
						0,0,   /* no flip */
						16*sx,16*sy);

				data = tilerom[offs+0x1000+bank];
				numtile = data&0xfff;
				color = (data&0xf000)>>12;
				m_gfxdecode->gfx(3)->opaque(*m_background_bitmap,m_background_bitmap->cliprect(),
						numtile,
						color,
						0,0,   /* no flip */
						16*sx,(16*sy)+256);

				sy++;
				if (sy==16) {sy=0; sx++;}
		}
	}

	copyscrollbitmap(bitmap,*m_background_bitmap,1,&scrollx,1,&scrolly,cliprect);
	m_bg_tilemap0->draw(screen, bitmap, cliprect, 0,0);

	nmk16_draw_sprites(bitmap,cliprect);

	m_tx_tilemap->draw(screen, bitmap, cliprect, 0,0);
	return 0;
}

int nmk16_state::nmk16_bg_sprswap_tx_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int bittbl[8])
{
	m_tx_tilemap->set_scrollx(0,-m_videoshift);

	m_bg_tilemap0->draw(screen, bitmap, cliprect, 0,0);

	nmk16_draw_sprites_swap(bitmap,cliprect, bittbl);

	m_tx_tilemap->draw(screen, bitmap, cliprect, 0,0);
	return 0;
}

int nmk16_state::nmk16_bg_sprswapflip_tx_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int bittbl[8])
{
	m_tx_tilemap->set_scrollx(0,-m_videoshift);

	m_bg_tilemap0->draw(screen, bitmap, cliprect, 0,0);

	nmk16_draw_sprites_swap_flipsupported(bitmap,cliprect, bittbl);

	m_tx_tilemap->draw(screen, bitmap, cliprect, 0,0);
	return 0;
}

int nmk16_state::nmk16_complexbg_sprswap_tx_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int bittbl[8])
{
	m_tx_tilemap->set_scrollx(0,-m_videoshift);

	// the hardware supports per-scanline X *and* Y scroll which isn't
	// supported by tilemaps so we have to draw the tilemap one line at a time
	if (!m_simple_scroll)
	{
	int i=16;
	int y1;
	rectangle bgclip = cliprect;
	y1 = cliprect.min_y;
		while (y1 <= cliprect.max_y)
		{
			int const yscroll = m_gunnail_scrollramy[0] + m_gunnail_scrollramy[y1];
			int tilemap_bank_select;
			tilemap_t* bg_tilemap = m_bg_tilemap0;

			bgclip.min_y = y1;
			bgclip.max_y = y1;


			tilemap_bank_select = (m_gunnail_scrollram[0]&0x3000)>>12;
			switch (tilemap_bank_select)
			{
				case 0: if (m_bg_tilemap0) bg_tilemap = m_bg_tilemap0; break;
				case 1: if (m_bg_tilemap1) bg_tilemap = m_bg_tilemap1; break;
				case 2: if (m_bg_tilemap2) bg_tilemap = m_bg_tilemap2; break;
				case 3: if (m_bg_tilemap3) bg_tilemap = m_bg_tilemap3; break;
			}

			bg_tilemap->set_scroll_rows(512);

			bg_tilemap->set_scrolly(0, yscroll);
			bg_tilemap->set_scrollx((i + yscroll) & 0x1ff, m_gunnail_scrollram[0] + m_gunnail_scrollram[i] - m_videoshift);

			bg_tilemap->draw(screen, bitmap, bgclip, 0,0);

			y1++;
			i++;
		}
	}
	else
	{
		UINT16 yscroll = ((m_gunnail_scrollram[2]&0xff)<<8) | ((m_gunnail_scrollram[3]&0xff)<<0);
		UINT16 xscroll = ((m_gunnail_scrollram[0]&0xff)<<8) | ((m_gunnail_scrollram[1]&0xff)<<0);
		int tilemap_bank_select;
		tilemap_t* bg_tilemap = m_bg_tilemap0;

		//popmessage( "scroll %04x, %04x", yscroll,xscroll);

		tilemap_bank_select = (xscroll&0x3000)>>12;
		switch (tilemap_bank_select)
		{
			case 0: if (m_bg_tilemap0) bg_tilemap = m_bg_tilemap0; break;
			case 1: if (m_bg_tilemap1) bg_tilemap = m_bg_tilemap1; break;
			case 2: if (m_bg_tilemap2) bg_tilemap = m_bg_tilemap2; break;
			case 3: if (m_bg_tilemap3) bg_tilemap = m_bg_tilemap3; break;
		}

		bg_tilemap->set_scroll_rows(1);

		bg_tilemap->set_scrolly(0, yscroll);
		bg_tilemap->set_scrollx(0, xscroll - m_videoshift);

		bg_tilemap->draw(screen, bitmap, cliprect, 0,0);
	}

	nmk16_draw_sprites_swap(bitmap,cliprect, bittbl);

	m_tx_tilemap->draw(screen, bitmap, cliprect, 0,0);
	return 0;
}

/***************************************************************************


                            Screen update functions


***************************************************************************/

UINT32 nmk16_state::screen_update_macross(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	return nmk16_bg_spr_tx_update(screen, bitmap, cliprect);
}

UINT32 nmk16_state::screen_update_manybloc(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	return nmk16_bg_sprflip_tx_update(screen, bitmap, cliprect);
}

UINT32 nmk16_state::screen_update_tharrier(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	/* I think the protection device probably copies this to the regs... */
	UINT16 tharrier_scroll = m_mainram[0x9f00/2];

	m_bg_tilemap0->set_scrollx(0,tharrier_scroll);

	return nmk16_bg_sprflip_tx_update(screen, bitmap, cliprect);
}

UINT32 nmk16_state::screen_update_tdragon2(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	static int bittbl[8] = {
	4, 6, 5, 7, 3, 2, 1, 0
	};

	return nmk16_complexbg_sprswap_tx_update(screen, bitmap, cliprect, bittbl);
}

UINT32 nmk16_state::screen_update_gunnail(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	static int bittbl[8] = {
	7, 6, 5, 4, 3, 2, 1, 0
	};

	return nmk16_complexbg_sprswap_tx_update(screen, bitmap, cliprect, bittbl);
}

UINT32 nmk16_state::screen_update_bioship(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	return nmk16_bioshipbg_sprflip_tx_update(screen, bitmap, cliprect);
}

UINT32 nmk16_state::screen_update_strahl(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	return nmk16_bg_fg_spr_tx_update(screen, bitmap, cliprect);
}

UINT32 nmk16_state::screen_update_bjtwin(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	return nmk16_bg_spr_update(screen, bitmap, cliprect);
}


/***************************************************************************


                            Video Hardware Init


***************************************************************************/

VIDEO_START_MEMBER(nmk16_state,afega)
{
	m_spriteram_old = auto_alloc_array_clear(machine(), UINT16, 0x1000/2);
	m_spriteram_old2 = auto_alloc_array_clear(machine(), UINT16, 0x1000/2);

	m_bg_tilemap0 = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(nmk16_state::macross_get_bg0_tile_info),this), tilemap_mapper_delegate(FUNC(nmk16_state::afega_tilemap_scan_pages),this),
								16,16,
								TILES_PER_PAGE_X*PAGES_PER_TMAP_X,TILES_PER_PAGE_Y*PAGES_PER_TMAP_Y);

	m_tx_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(nmk16_state::macross_get_tx_tile_info),this), TILEMAP_SCAN_COLS,
								8,8,
								32,32);

	m_tx_tilemap->set_transparent_pen(0xf);
}


VIDEO_START_MEMBER(nmk16_state,grdnstrm)
{
	m_spriteram_old = auto_alloc_array_clear(machine(), UINT16, 0x1000/2);
	m_spriteram_old2 = auto_alloc_array_clear(machine(), UINT16, 0x1000/2);


	m_bg_tilemap0 = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(nmk16_state::get_tile_info_0_8bit),this), tilemap_mapper_delegate(FUNC(nmk16_state::afega_tilemap_scan_pages),this),
								16,16,
								TILES_PER_PAGE_X*PAGES_PER_TMAP_X,TILES_PER_PAGE_Y*PAGES_PER_TMAP_Y);

	m_tx_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(nmk16_state::macross_get_tx_tile_info),this), TILEMAP_SCAN_COLS,
								8,8,
								32,32);

	m_tx_tilemap->set_transparent_pen(0xf);
}


VIDEO_START_MEMBER(nmk16_state,firehawk)
{
	m_spriteram_old = auto_alloc_array_clear(machine(), UINT16, 0x1000/2);
	m_spriteram_old2 = auto_alloc_array_clear(machine(), UINT16, 0x1000/2);


	m_bg_tilemap0 = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(nmk16_state::get_tile_info_0_8bit),this), tilemap_mapper_delegate(FUNC(nmk16_state::afega_tilemap_scan_pages),this),
								16,16,
								TILES_PER_PAGE_X*PAGES_PER_TMAP_X,TILES_PER_PAGE_Y*PAGES_PER_TMAP_Y);

	m_tx_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(nmk16_state::macross_get_tx_tile_info),this), TILEMAP_SCAN_COLS,
								8,8,
								32,32);

	m_tx_tilemap->set_transparent_pen(0xf);
}


/***************************************************************************


                                Screen Drawing


***************************************************************************/

void nmk16_state::video_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect,
	int dsw_flipscreen,         // 1 = Horizontal and vertical screen flip are hardwired to 2 dip switches
	int xoffset, int yoffset,   // bg_tilemap0 offsets
	int attr_mask               // "sprite active" mask
	)
{
	if (dsw_flipscreen)
	{
		flip_screen_x_set(~ioport("DSW1")->read() & 0x0100);
		flip_screen_y_set(~ioport("DSW1")->read() & 0x0200);
	}


	m_bg_tilemap0->set_scrollx(0, m_afega_scroll_0[1] + xoffset);
	m_bg_tilemap0->set_scrolly(0, m_afega_scroll_0[0] + yoffset);

	m_tx_tilemap->set_scrollx(0, m_afega_scroll_1[1]);
	m_tx_tilemap->set_scrolly(0, m_afega_scroll_1[0]);


	m_bg_tilemap0->draw(screen, bitmap, cliprect, 0,0);

	nmk16_draw_sprites_flipsupported(bitmap,cliprect);

	m_tx_tilemap->draw(screen, bitmap, cliprect, 0,0);
}

void nmk16_state::redhawki_video_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	m_bg_tilemap0->set_scrollx(0, m_afega_scroll_1[0]&0xff);
	m_bg_tilemap0->set_scrolly(0, m_afega_scroll_1[1]&0xff);

	m_bg_tilemap0->draw(screen, bitmap, cliprect, 0,0);

	nmk16_draw_sprites_flipsupported(bitmap,cliprect);
}

UINT32 nmk16_state::screen_update_afega(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)   { video_update(screen,bitmap,cliprect, 1, -0x100,+0x000, 0x0001);  return 0; }
UINT32 nmk16_state::screen_update_bubl2000(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect){ video_update(screen,bitmap,cliprect, 0, -0x100,+0x000, 0x0001);  return 0; } // no flipscreen support, I really would confirmation from the schematics
UINT32 nmk16_state::screen_update_redhawkb(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect){ video_update(screen,bitmap,cliprect, 0, +0x000,+0x100, 0x0001);  return 0; }
UINT32 nmk16_state::screen_update_redhawki(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect){ redhawki_video_update(screen,bitmap,cliprect); return 0;} // strange scroll regs

UINT32 nmk16_state::screen_update_firehawk(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap0->set_scrolly(0, m_afega_scroll_1[1] + 0x100);
	m_bg_tilemap0->set_scrollx(0, m_afega_scroll_0[1] - 0x100);

	m_bg_tilemap0->draw(screen, bitmap, cliprect, 0,0);

	nmk16_draw_sprites_flipsupported(bitmap,cliprect);

	m_tx_tilemap->draw(screen, bitmap, cliprect, 0,0);
	return 0;
}
