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

/*
#define TILES_PER_PAGE_X    (0x10)
#define TILES_PER_PAGE_Y    (0x10)
#define PAGES_PER_TMAP_X    (0x10)
#define PAGES_PER_TMAP_Y    (0x02)
*/

TILEMAP_MAPPER_MEMBER(nmk16_state::tilemap_scan_pages)
{
	return  (row & 0xf) | ((col & 0xff) << 4) | ((row & 0x10) << 8);
}

template<int Layer, int Gfx>
TILE_GET_INFO_MEMBER(nmk16_state::common_get_bg_tile_info)
{
	int code = m_nmk_bgvideoram[Layer][(m_tilerambank << 13)|tile_index];
	SET_TILE_INFO_MEMBER(Gfx,(code & 0xfff) | (m_bgbank << 12),code >> 12,0);
}

TILE_GET_INFO_MEMBER(nmk16_state::common_get_tx_tile_info)
{
	int code = m_nmk_txvideoram[tile_index];
	SET_TILE_INFO_MEMBER(0,
			code & 0xfff,
			code >> 12,
			0);
}

TILE_GET_INFO_MEMBER(nmk16_state::bioship_get_bg_tile_info)
{
	int code = m_tilemap_rom[(m_bioship_background_bank << 13) | tile_index]; // ROM Based
	SET_TILE_INFO_MEMBER(3,(code & 0xfff),code >> 12,0);
}

TILE_GET_INFO_MEMBER(nmk16_state::bjtwin_get_bg_tile_info)
{
	int code = m_nmk_bgvideoram[0][tile_index];
	int bank = (code & 0x800) ? 1 : 0;
	SET_TILE_INFO_MEMBER(bank,
			(code & 0x7ff) + ((bank) ? (m_bgbank << 11) : 0),
			code >> 12,
			0);
}

TILE_GET_INFO_MEMBER(nmk16_state::get_tile_info_0_8bit)
{
	uint16_t code = m_nmk_bgvideoram[0][tile_index];
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
	m_spriteram_old = make_unique_clear<uint16_t[]>(0x1000/2);
	m_spriteram_old2 = make_unique_clear<uint16_t[]>(0x1000/2);

	m_videoshift = 0;        /* 256x224 screen, no shift */
	m_tilerambank = 0;
	m_sprclk = 0;

	save_pointer(NAME(m_spriteram_old), 0x1000/2);
	save_pointer(NAME(m_spriteram_old2), 0x1000/2);
	save_item(NAME(m_bgbank));
	save_item(NAME(m_mustang_bg_xscroll));
	save_item(NAME(m_scroll[0]));
	save_item(NAME(m_scroll[1]));
	save_item(NAME(m_vscroll));
	save_item(NAME(m_tilerambank));
	save_item(NAME(m_sprclk));
}


VIDEO_START_MEMBER(nmk16_state,bioship)
{
	m_sprlimit = 384 * 263;
	// ROM Based Tilemap
	m_bg_tilemap[0] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(FUNC(nmk16_state::bioship_get_bg_tile_info),this), tilemap_mapper_delegate(FUNC(nmk16_state::tilemap_scan_pages),this),16,16,256,32);
	m_bg_tilemap[1] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(&nmk16_state::common_get_bg_tile_info<1, 1>, "bg1_gfx1",this), tilemap_mapper_delegate(FUNC(nmk16_state::tilemap_scan_pages),this),16,16,256,32);
	m_tx_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(FUNC(nmk16_state::common_get_tx_tile_info),this),TILEMAP_SCAN_COLS,8,8,32,32);

	m_bg_tilemap[1]->set_transparent_pen(15);
	m_tx_tilemap->set_transparent_pen(15);

	nmk16_video_init();
	m_bioship_background_bank=0;
	save_item(NAME(m_bioship_background_bank));
}

VIDEO_START_MEMBER(nmk16_state,macross)
{
	m_sprlimit = 384 * 263;
	m_bg_tilemap[0] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(&nmk16_state::common_get_bg_tile_info<0, 1>, "bg0_gfx1",this), tilemap_mapper_delegate(FUNC(nmk16_state::tilemap_scan_pages),this),16,16,256,32);
	m_tx_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(FUNC(nmk16_state::common_get_tx_tile_info),this),TILEMAP_SCAN_COLS,8,8,32,32);

	m_tx_tilemap->set_transparent_pen(15);

	nmk16_video_init();
}

VIDEO_START_MEMBER(nmk16_state,strahl)
{
	VIDEO_START_CALL_MEMBER( macross );
	m_bg_tilemap[1] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(&nmk16_state::common_get_bg_tile_info<1, 3>, "bg1_gfx3",this), tilemap_mapper_delegate(FUNC(nmk16_state::tilemap_scan_pages),this),16,16,256,32);
	m_bg_tilemap[1]->set_transparent_pen(15);

	m_sprdma_base = 0xf000;
}

VIDEO_START_MEMBER(nmk16_state,macross2)
{
	m_sprlimit = 512 * 263; // not verified
	m_bg_tilemap[0] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(&nmk16_state::common_get_bg_tile_info<0, 1>, "bg0_gfx1",this), tilemap_mapper_delegate(FUNC(nmk16_state::tilemap_scan_pages),this),16,16,256,32);
	m_tx_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(FUNC(nmk16_state::common_get_tx_tile_info),this),TILEMAP_SCAN_COLS,8,8,64,32);

	m_tx_tilemap->set_transparent_pen(15);

	nmk16_video_init();
	m_videoshift = 64;  /* 384x224 screen, leftmost 64 pixels have to be retrieved */
						/* from the other side of the tilemap (!) */
}

VIDEO_START_MEMBER(nmk16_state,gunnail)
{
	VIDEO_START_CALL_MEMBER( macross2 );
	m_bg_tilemap[0]->set_scroll_rows(512);
}

VIDEO_START_MEMBER(nmk16_state,bjtwin)
{
	m_sprlimit = 512 * 263; // not verified
	m_bg_tilemap[0] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(FUNC(nmk16_state::bjtwin_get_bg_tile_info),this),TILEMAP_SCAN_COLS,8,8,64,32);

	nmk16_video_init();
	m_videoshift = 64;  /* 384x224 screen, leftmost 64 pixels have to be retrieved */
						/* from the other side of the tilemap (!) */
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

	m_bg_tilemap[0]->set_scrollx(0,m_mustang_bg_xscroll - m_videoshift);
}

WRITE16_MEMBER(nmk16_state::vandyke_scroll_w)
{
	m_vscroll[offset] = data;

	m_bg_tilemap[0]->set_scrollx(0,m_vscroll[0] * 256 + (m_vscroll[1] >> 8));
	m_bg_tilemap[0]->set_scrolly(0,m_vscroll[2] * 256 + (m_vscroll[3] >> 8));
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

	m_bg_tilemap[0]->set_scrollx(0,m_vscroll[0] * 256 + (m_vscroll[1] >> 8));
	m_bg_tilemap[0]->set_scrolly(0,m_vscroll[2] * 256 + (m_vscroll[3] >> 8));
}

WRITE16_MEMBER(nmk16_state::manybloc_scroll_w)
{
	COMBINE_DATA(&m_gunnail_scrollram[offset]);

	m_bg_tilemap[0]->set_scrollx(0,m_gunnail_scrollram[0x82/2]-m_videoshift);
	m_bg_tilemap[0]->set_scrolly(0,m_gunnail_scrollram[0xc2/2]);
}

WRITE8_MEMBER(nmk16_state::nmk_flipscreen_w)
{
	flip_screen_set(data & 0x01);
}

WRITE8_MEMBER(nmk16_state::nmk_tilebank_w)
{
	if (m_bgbank != data)
	{
		m_bgbank = data;
		for (int layer = 0; layer < 2; layer++)
			if (m_bg_tilemap[layer]) m_bg_tilemap[layer]->mark_all_dirty();

	}
}

WRITE16_MEMBER(nmk16_state::raphero_scroll_w)
{
	COMBINE_DATA(&m_gunnail_scrollram[offset]);
	if ((m_nmk_bgvideoram[0].bytes() > 0x4000) && (offset == 0))
	{
		int newbank = (m_gunnail_scrollram[0] >> 12) & ((m_nmk_bgvideoram[0].bytes() >> 14)-1);
		if (m_tilerambank != newbank)
		{
			m_tilerambank = newbank;
		}
	}
}

WRITE8_MEMBER(nmk16_state::bioship_bank_w)
{
	if (m_bioship_background_bank != data)
	{
		m_bioship_background_bank = data;
		m_bg_tilemap[0]->mark_all_dirty();
	}
}

/***************************************************************************

  Display refresh

***************************************************************************/



// manybloc uses extra flip bits on the sprites, but these break other games

inline void nmk16_state::nmk16_draw_sprite(bitmap_ind16 &bitmap, const rectangle &cliprect, uint16_t *spr)
{
	if(!(spr[0] & 0x0001))
		return;

	int sx    = (spr[4] & 0x1ff) + m_videoshift;
	int sy    =  spr[6] & 0x1ff;
	int code  =  spr[3];
	int color =  spr[7];
	int w     =  spr[1] & 0x00f;
	int h     = (spr[1] & 0x0f0) >> 4;
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
			((x + 16) & 0x1ff) - 16,sy & 0x1ff,15);
		m_sprclk += 128; // 128 clock per each 16x16 tile
		if (m_sprclk >= m_sprlimit)
			return;

		code++;
		x += delta;
		} while (--xx >= 0);

		sy += delta;
	} while (--yy >= 0);
}

inline void nmk16_state::nmk16_draw_sprite_flipsupported(bitmap_ind16 &bitmap, const rectangle &cliprect, uint16_t *spr)
{
	if(!(spr[0] & 0x0001))
		return;

	int sx    = (spr[4] & 0x1ff) + m_videoshift;
	int sy    =  spr[6] & 0x1ff;
	int code  =  spr[3];
	int color =  spr[7];
	int w     =  spr[1] & 0x00f;
	int h     = (spr[1] & 0x0f0) >> 4;
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
			((x + 16) & 0x1ff) - 16,sy & 0x1ff,15);
		m_sprclk += 128; // 128 clock per each 16x16 tile
		if (m_sprclk >= m_sprlimit)
			return;

		code++;
		x += delta * (flipx ? -1 : 1);
		} while (--xx >= 0);
		sy += delta * (flipy ? -1 : 1);
	} while (--yy >= 0);
}

void nmk16_state::nmk16_draw_sprites_swap(bitmap_ind16 &bitmap, const rectangle &cliprect, int *bittbl)
{
	m_sprclk = 0;
	int i;

	for (i = 0; i < 0x100; i++)
	{
		m_sprclk += 16; // 16 clock per each reading
		if (m_sprclk >= m_sprlimit)
			return;

		int spr = bitswap<8>(i, bittbl[0], bittbl[1], bittbl[2], bittbl[3], bittbl[4], bittbl[5], bittbl[6], bittbl[7]);
		nmk16_draw_sprite(bitmap, cliprect, m_spriteram_old2.get() + (spr * 16/2));
	}
}

void nmk16_state::nmk16_draw_sprites_swap_flipsupported(bitmap_ind16 &bitmap, const rectangle &cliprect, int *bittbl)
{
	m_sprclk = 0;
	int i;

	for ( i = 0; i < 0x100; i++ )
	{
		m_sprclk += 16; // 16 clock per each reading
		if (m_sprclk >= m_sprlimit)
			return;

		int spr = bitswap<8>(i, bittbl[0], bittbl[1], bittbl[2], bittbl[3], bittbl[4], bittbl[5], bittbl[6], bittbl[7]);
		nmk16_draw_sprite_flipsupported(bitmap, cliprect, m_spriteram_old2.get() + (spr * 16/2));
	}
}

void nmk16_state::nmk16_draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_sprclk = 0;
	int offs;

	for (offs = 0; offs < 0x1000/2; offs += 8)
	{
		m_sprclk += 16; // 16 clock per each reading
		if (m_sprclk >= m_sprlimit)
			return;

		nmk16_draw_sprite(bitmap, cliprect, m_spriteram_old2.get() + offs);
	}
}

void nmk16_state::nmk16_draw_sprites_flipsupported(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_sprclk = 0;
	int offs;

	for (offs = 0; offs < 0x1000/2; offs += 8)
	{
		m_sprclk += 16; // 16 clock per each reading
		if (m_sprclk >= m_sprlimit)
			return;

		nmk16_draw_sprite_flipsupported(bitmap, cliprect, m_spriteram_old2.get() + offs);
	}
}

/***************************************************************************


                            Generic Screen Updates


***************************************************************************/

void nmk16_state::nmk16_bg_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int layer)
{
	if (m_gunnail_scrollram && m_gunnail_scrollramy)
	{
		// the hardware supports per-scanline X *and* Y scroll which isn't
		// supported by tilemaps so we have to draw the tilemap one line at a time
		int i=16;
		int y1;
		rectangle bgclip = cliprect;
		y1 = cliprect.min_y;
		while (y1 <= cliprect.max_y)
		{
			int const yscroll = m_gunnail_scrollramy[0] + m_gunnail_scrollramy[y1];

			bgclip.min_y = y1;
			bgclip.max_y = y1;

			m_bg_tilemap[layer]->set_scroll_rows(512);

			m_bg_tilemap[layer]->set_scrolly(0, yscroll);
			m_bg_tilemap[layer]->set_scrollx((i + yscroll) & 0x1ff, m_gunnail_scrollram[0] + m_gunnail_scrollram[i] - m_videoshift);

			m_bg_tilemap[layer]->draw(screen, bitmap, bgclip, 0,0);

			y1++;
			i++;
		}
	}
	else
	{
		m_bg_tilemap[layer]->draw(screen, bitmap, cliprect, 0,0);
	}
}

void nmk16_state::nmk16_tx_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_tx_tilemap->set_scrollx(0,-m_videoshift);
	m_tx_tilemap->draw(screen, bitmap, cliprect, 0,0);
}

/***************************************************************************


                            Screen update functions


***************************************************************************/

uint32_t nmk16_state::screen_update_macross(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	nmk16_bg_update(screen,bitmap,cliprect,0);
	nmk16_draw_sprites(bitmap,cliprect);
	nmk16_tx_update(screen,bitmap,cliprect);
	return 0;
}

uint32_t nmk16_state::screen_update_manybloc(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	nmk16_bg_update(screen,bitmap,cliprect,0);
	nmk16_draw_sprites_flipsupported(bitmap,cliprect);
	nmk16_tx_update(screen,bitmap,cliprect);
	return 0;
}

uint32_t nmk16_state::screen_update_tharrier(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	/* I think the protection device probably copies this to the regs... */
	uint16_t tharrier_scroll = m_mainram[0x9f00/2];

	m_bg_tilemap[0]->set_scrollx(0,tharrier_scroll);

	nmk16_bg_update(screen,bitmap,cliprect,0);
	nmk16_draw_sprites_flipsupported(bitmap,cliprect);
	nmk16_tx_update(screen,bitmap,cliprect);
	return 0;
}

uint32_t nmk16_state::screen_update_tdragon2(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	static int bittbl[8] = {
	4, 6, 5, 7, 3, 2, 1, 0
	};

	nmk16_bg_update(screen,bitmap,cliprect,0);
	nmk16_draw_sprites_swap(bitmap,cliprect, bittbl);
	nmk16_tx_update(screen,bitmap,cliprect);
	return 0;
}

uint32_t nmk16_state::screen_update_strahl(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	nmk16_bg_update(screen,bitmap,cliprect,0);
	nmk16_bg_update(screen,bitmap,cliprect,1);
	nmk16_draw_sprites(bitmap,cliprect);
	nmk16_tx_update(screen,bitmap,cliprect);
	return 0;
}

uint32_t nmk16_state::screen_update_bjtwin(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap[0]->set_scrollx(0,-m_videoshift);

	nmk16_bg_update(screen,bitmap,cliprect,0);
	nmk16_draw_sprites(bitmap,cliprect);
	return 0;
}


/***************************************************************************


                            Video Hardware Init


***************************************************************************/

VIDEO_START_MEMBER(nmk16_state,afega)
{
	m_sprlimit = 384 * 263;
	m_spriteram_old = make_unique_clear<uint16_t[]>(0x1000/2);
	m_spriteram_old2 = make_unique_clear<uint16_t[]>(0x1000/2);

	m_bg_tilemap[0] = &machine().tilemap().create(
			*m_gfxdecode,
			tilemap_get_info_delegate(&nmk16_state::common_get_bg_tile_info<0, 1>, "bg0_gfx1",this),
			tilemap_mapper_delegate(FUNC(nmk16_state::tilemap_scan_pages),this),
			16,16, 256,32);

	m_tx_tilemap = &machine().tilemap().create(
			*m_gfxdecode,
			tilemap_get_info_delegate(FUNC(nmk16_state::common_get_tx_tile_info),this),
			TILEMAP_SCAN_COLS,
			8,8, 32,32);

	m_tx_tilemap->set_transparent_pen(0xf);
	save_pointer(NAME(m_spriteram_old), 0x1000/2);
	save_pointer(NAME(m_spriteram_old2), 0x1000/2);
}


VIDEO_START_MEMBER(nmk16_state,grdnstrm)
{
	m_sprlimit = 384 * 263;
	m_spriteram_old = make_unique_clear<uint16_t[]>(0x1000/2);
	m_spriteram_old2 = make_unique_clear<uint16_t[]>(0x1000/2);

	m_bg_tilemap[0] = &machine().tilemap().create(
			*m_gfxdecode,
			tilemap_get_info_delegate(FUNC(nmk16_state::get_tile_info_0_8bit),this),
			tilemap_mapper_delegate(FUNC(nmk16_state::tilemap_scan_pages),this),
			16,16, 256,32);

	m_tx_tilemap = &machine().tilemap().create(
			*m_gfxdecode,
			tilemap_get_info_delegate(FUNC(nmk16_state::common_get_tx_tile_info),this),
			TILEMAP_SCAN_COLS,
			8,8, 32,32);

	m_tx_tilemap->set_transparent_pen(0xf);
	save_pointer(NAME(m_spriteram_old), 0x1000/2);
	save_pointer(NAME(m_spriteram_old2), 0x1000/2);
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


	m_bg_tilemap[0]->set_scrollx(0, m_afega_scroll[0][1] + xoffset);
	m_bg_tilemap[0]->set_scrolly(0, m_afega_scroll[0][0] + yoffset);

	m_tx_tilemap->set_scrollx(0, m_afega_scroll[1][1]);
	m_tx_tilemap->set_scrolly(0, m_afega_scroll[1][0]);


	m_bg_tilemap[0]->draw(screen, bitmap, cliprect, 0,0);

	nmk16_draw_sprites_flipsupported(bitmap,cliprect);

	m_tx_tilemap->draw(screen, bitmap, cliprect, 0,0);
}

void nmk16_state::redhawki_video_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	m_bg_tilemap[0]->set_scrollx(0, m_afega_scroll[1][0]&0xff);
	m_bg_tilemap[0]->set_scrolly(0, m_afega_scroll[1][1]&0xff);

	m_bg_tilemap[0]->draw(screen, bitmap, cliprect, 0,0);

	nmk16_draw_sprites_flipsupported(bitmap,cliprect);
}

uint32_t nmk16_state::screen_update_afega(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)   { video_update(screen,bitmap,cliprect, 1, -0x100,+0x000, 0x0001);  return 0; }
uint32_t nmk16_state::screen_update_bubl2000(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect){ video_update(screen,bitmap,cliprect, 0, -0x100,+0x000, 0x0001);  return 0; } // no flipscreen support, I really would confirmation from the schematics
uint32_t nmk16_state::screen_update_redhawkb(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect){ video_update(screen,bitmap,cliprect, 0, +0x000,+0x100, 0x0001);  return 0; }
uint32_t nmk16_state::screen_update_redhawki(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect){ redhawki_video_update(screen,bitmap,cliprect); return 0;} // strange scroll regs

uint32_t nmk16_state::screen_update_firehawk(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap[0]->set_scrolly(0, m_afega_scroll[1][1] + 0x100);
	m_bg_tilemap[0]->set_scrollx(0, m_afega_scroll[0][1] - 0x100);

	m_bg_tilemap[0]->draw(screen, bitmap, cliprect, 0,0);

	nmk16_draw_sprites_flipsupported(bitmap,cliprect);

	m_tx_tilemap->draw(screen, bitmap, cliprect, 0,0);
	return 0;
}
