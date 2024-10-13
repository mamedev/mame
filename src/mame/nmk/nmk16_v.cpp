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
#include "nmk16.h"

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
	return (row & 0xf) | ((col & 0xff) << 4) | ((row & 0x10) << 8);
}

template<unsigned Layer, unsigned Gfx>
TILE_GET_INFO_MEMBER(nmk16_state::common_get_bg_tile_info)
{
	const u16 code = m_bgvideoram[Layer][(m_tilerambank << 13) | tile_index];
	tileinfo.set(Gfx, (code & 0xfff) | (m_bgbank << 12), code >> 12, 0);
}

TILE_GET_INFO_MEMBER(nmk16_state::common_get_tx_tile_info)
{
	const u16 code = m_txvideoram[tile_index];
	tileinfo.set(0, code & 0xfff, code >> 12, 0);
}

TILE_GET_INFO_MEMBER(nmk16_state::bioship_get_bg_tile_info)
{
	const u16 code = m_tilemap_rom[(m_bioship_background_bank << 13) | tile_index]; // ROM Based
	tileinfo.set(3, code & 0xfff, code >> 12, 0);
}

TILE_GET_INFO_MEMBER(nmk16_state::bjtwin_get_bg_tile_info)
{
	const u16 code = m_bgvideoram[0][tile_index];
	const u8 bank = BIT(code, 11);
	tileinfo.set(bank,
			(code & 0x7ff) + ((bank) ? (m_bgbank << 11) : 0),
			code >> 12,
			0);
}

TILE_GET_INFO_MEMBER(nmk16_state::powerins_get_bg_tile_info)
{
	const u16 code = m_bgvideoram[0][tile_index];
	tileinfo.set(1,
			(code & 0x07ff) | (m_bgbank << 11),
			((code & 0xf000) >> 12) | ((code & 0x0800) >> 7),
			0);
}


/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

void nmk16_state::video_init()
{
	m_spriteram_old = make_unique_clear<u16[]>(0x1000/2);
	m_spriteram_old2 = make_unique_clear<u16[]>(0x1000/2);

	m_tilerambank = 0;
	m_bgbank = 0;
	m_mustang_bg_xscroll = 0;

	save_pointer(NAME(m_spriteram_old), 0x1000/2);
	save_pointer(NAME(m_spriteram_old2), 0x1000/2);
	save_item(NAME(m_bgbank));
	save_item(NAME(m_mustang_bg_xscroll));
	save_item(NAME(m_scroll[0]));
	save_item(NAME(m_scroll[1]));
	save_item(NAME(m_vscroll));
	save_item(NAME(m_tilerambank));
}


VIDEO_START_MEMBER(nmk16_state, bioship)
{
	// ROM Based Tilemap
	m_bg_tilemap[0] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(nmk16_state::bioship_get_bg_tile_info)), tilemap_mapper_delegate(*this, FUNC(nmk16_state::tilemap_scan_pages)), 16, 16, 256, 32);
	m_bg_tilemap[1] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, NAME((&nmk16_state::common_get_bg_tile_info<1, 1>))), tilemap_mapper_delegate(*this, FUNC(nmk16_state::tilemap_scan_pages)), 16, 16, 256, 32);
	m_tx_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(nmk16_state::common_get_tx_tile_info)), TILEMAP_SCAN_COLS, 8, 8, 32, 32);

	m_bg_tilemap[1]->set_transparent_pen(15);
	m_tx_tilemap->set_transparent_pen(15);

	video_init();
	m_bioship_background_bank=0;
	save_item(NAME(m_bioship_background_bank));
	m_bg_tilemap[0]->set_scrolldx(92, 92);
	m_bg_tilemap[1]->set_scrolldx(92, 92);
	m_tx_tilemap->set_scrolldx(92, 92);
}

VIDEO_START_MEMBER(nmk16_state,macross)
{
	m_bg_tilemap[0] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, NAME((&nmk16_state::common_get_bg_tile_info<0, 1>))), tilemap_mapper_delegate(*this, FUNC(nmk16_state::tilemap_scan_pages)), 16, 16, 256, 32);
	m_bg_tilemap[1] = nullptr;
	m_tx_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(nmk16_state::common_get_tx_tile_info)), TILEMAP_SCAN_COLS, 8, 8, 32, 32);

	m_tx_tilemap->set_transparent_pen(15);

	video_init();
	m_bg_tilemap[0]->set_scrolldx(92, 92);
	m_tx_tilemap->set_scrolldx(92, 92);
}

VIDEO_START_MEMBER(nmk16_state,strahl)
{
	VIDEO_START_CALL_MEMBER(macross);
	m_bg_tilemap[1] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, NAME((&nmk16_state::common_get_bg_tile_info<1, 3>))), tilemap_mapper_delegate(*this, FUNC(nmk16_state::tilemap_scan_pages)), 16, 16, 256, 32);
	m_bg_tilemap[1]->set_transparent_pen(15);

	m_sprdma_base = 0xf000;
	m_bg_tilemap[1]->set_scrolldx(92, 92);
}

VIDEO_START_MEMBER(nmk16_state,macross2)
{
	m_bg_tilemap[0] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, NAME((&nmk16_state::common_get_bg_tile_info<0, 1>))), tilemap_mapper_delegate(*this, FUNC(nmk16_state::tilemap_scan_pages)), 16, 16, 256, 32);
	m_bg_tilemap[1] = nullptr;
	m_tx_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(nmk16_state::common_get_tx_tile_info)), TILEMAP_SCAN_COLS, 8, 8, 64, 32);

	m_tx_tilemap->set_transparent_pen(15);

	video_init();
	// 384x224 screen, leftmost 64 pixels have to be retrieved from the other side of the tilemap (!)
	m_bg_tilemap[0]->set_scrolldx(28+64, 28+64);
	m_tx_tilemap->set_scrolldx(28+64, 28+64);
}

VIDEO_START_MEMBER(nmk16_state,gunnail)
{
	VIDEO_START_CALL_MEMBER(macross2);
	m_bg_tilemap[0]->set_scroll_rows(512);
}

VIDEO_START_MEMBER(nmk16_state, bjtwin)
{
	m_bg_tilemap[0] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(nmk16_state::bjtwin_get_bg_tile_info)), TILEMAP_SCAN_COLS, 8, 8, 64, 32);
	m_bg_tilemap[1] = nullptr;

	video_init();
	// 384x224 screen, leftmost 64 pixels have to be retrieved from the other side of the tilemap (!)
	m_bg_tilemap[0]->set_scrolldx(28+64, 28+64);
}

/***************************************************************************
                          [ Tiles Format VRAM 0]
Offset:

0.w     fedc ---- ---- ----     Color Low  Bits
        ---- b--- ---- ----     Color High Bit
        ---- -a98 7654 3210     Code (Banked)

                          [ Tiles Format VRAM 1]
Offset:

0.w     fedc ---- ---- ----     Color
        ---- ba98 7654 3210     Code

***************************************************************************/

VIDEO_START_MEMBER(nmk16_state, powerins)
{
	m_bg_tilemap[0] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(nmk16_state::powerins_get_bg_tile_info)), tilemap_mapper_delegate(*this, FUNC(nmk16_state::tilemap_scan_pages)), 16, 16, 256, 32);
	m_bg_tilemap[1] = nullptr;
	m_tx_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(nmk16_state::common_get_tx_tile_info)), TILEMAP_SCAN_COLS, 8, 8, 64, 32);

	m_tx_tilemap->set_transparent_pen(15);

	video_init();
	// 320x224 screen, leftmost 32 pixels have to be retrieved from the other side of the tilemap (!)
	m_bg_tilemap[0]->set_scrolldx(60+32, 60+32);
	m_tx_tilemap->set_scrolldx(60+32, 60+32);
}

void nmk16_state::mustang_scroll_w(u16 data)
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

	m_bg_tilemap[0]->set_scrollx(0,m_mustang_bg_xscroll);
}

void nmk16_state::bjtwin_scroll_w(offs_t offset, u8 data)
{
	m_bg_tilemap[0]->set_scrolly(0,-data);
}

void nmk16_state::vandyke_scroll_w(offs_t offset, u16 data)
{
	m_vscroll[offset] = data;

	m_bg_tilemap[0]->set_scrollx(0,m_vscroll[0] * 256 + (m_vscroll[1] >> 8));
	m_bg_tilemap[0]->set_scrolly(0,m_vscroll[2] * 256 + (m_vscroll[3] >> 8));
}

void nmk16_state::vandykeb_scroll_w(offs_t offset, u16 data, u16 mem_mask)
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

void nmk16_state::manybloc_scroll_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_gunnail_scrollram[offset]);

	m_bg_tilemap[0]->set_scrollx(0,m_gunnail_scrollram[0x82/2]);
	m_bg_tilemap[0]->set_scrolly(0,m_gunnail_scrollram[0xc2/2]);
}

void nmk16_state::flipscreen_w(u8 data)
{
	flip_screen_set(data & 0x01);
	m_spritegen->set_flip_screen(flip_screen());
}

// vandyke writes a 0 value when flip screen is enabled, contrary to rest of games that write 1
void nmk16_state::vandyke_flipscreen_w(u8 data)
{
	flipscreen_w(~data);
}

void nmk16_state::tilebank_w(u8 data)
{
	if (m_bgbank != data)
	{
		m_bgbank = data;
		for (int layer = 0; layer < 2; layer++)
			if (m_bg_tilemap[layer]) m_bg_tilemap[layer]->mark_all_dirty();

	}
}

void nmk16_state::raphero_scroll_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_gunnail_scrollram[offset]);
	if ((m_bgvideoram[0].bytes() > 0x4000) && (offset == 0))
	{
		int newbank = (m_gunnail_scrollram[0] >> 12) & ((m_bgvideoram[0].bytes() >> 14) - 1);
		if (m_tilerambank != newbank)
		{
			m_tilerambank = newbank;
			for (int layer = 0; layer < 2; layer++)
			{
				if (m_bg_tilemap[layer])
					m_bg_tilemap[layer]->mark_all_dirty();
			}
		}
	}
}

void nmk16_state::bioship_bank_w(u8 data)
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


void nmk16_state::get_colour_4bit(u32 &colour, u32 &pri_mask)
{
	colour &= 0xf;
	pri_mask |= GFX_PMASK_2; // under foreground
}

void nmk16_state::get_colour_5bit(u32 &colour, u32 &pri_mask)
{
	colour &= 0x1f;
	pri_mask |= GFX_PMASK_2; // under foreground
}

void nmk16_state::get_colour_6bit(u32 &colour, u32 &pri_mask)
{
	colour &= 0x3f;
	pri_mask |= GFX_PMASK_2; // under foreground
}

// manybloc uses extra flip bits on the sprites, but these break other games

void nmk16_state::get_sprite_flip(u16 attr, int &flipx, int &flipy, int &code)
{
	flipy = (attr & 0x200) >> 9;
	flipx = (attr & 0x100) >> 8;
}

void nmk16_state::get_flip_extcode_powerins(u16 attr, int &flipx, int &flipy, int &code)
{
	flipx = (attr & 0x1000) >> 12;
	code = (code & 0x7fff) | ((attr & 0x100) << 7);
}

void nmk16_state::draw_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, u16 *src)
{
	m_spritegen->draw_sprites(screen, bitmap, cliprect, m_gfxdecode->gfx(2), src, 0x1000 / 2);
}

/***************************************************************************


                            Generic Screen Updates


***************************************************************************/

void nmk16_state::bg_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int layer)
{
	if (m_gunnail_scrollram && m_gunnail_scrollramy)
	{
		// the hardware supports per-scanline X *and* Y scroll which isn't
		// supported by tilemaps so we have to draw the tilemap one line at a time
		int i = 16;
		rectangle bgclip = cliprect;
		int y1 = cliprect.min_y;
		while (y1 <= cliprect.max_y)
		{
			const int yscroll = m_gunnail_scrollramy[0] + m_gunnail_scrollramy[y1];

			bgclip.min_y = y1;
			bgclip.max_y = y1;

			m_bg_tilemap[layer]->set_scroll_rows(512);

			m_bg_tilemap[layer]->set_scrolly(0, yscroll);
			m_bg_tilemap[layer]->set_scrollx((i + yscroll) & 0x1ff, m_gunnail_scrollram[0] + m_gunnail_scrollram[i]);

			m_bg_tilemap[layer]->draw(screen, bitmap, bgclip, 0, 1);

			y1++;
			i++;
		}
	}
	else
	{
		m_bg_tilemap[layer]->draw(screen, bitmap, cliprect, 0, 1);
	}
}

void nmk16_state::tx_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_tx_tilemap->draw(screen, bitmap, cliprect, 0, 2);
}

/***************************************************************************


                            Screen update functions


***************************************************************************/

u32 nmk16_state::screen_update_macross(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	screen.priority().fill(0, cliprect);
	bg_update(screen, bitmap, cliprect, 0);
	tx_update(screen, bitmap, cliprect);
	draw_sprites(screen, bitmap, cliprect, m_spriteram_old2.get());
	return 0;
}

u32 nmk16_state::screen_update_tharrier(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	screen.priority().fill(0, cliprect);
	/* I think the protection device probably copies this to the regs... */
	u16 tharrier_scroll = m_mainram[0x9f00/2];

	m_bg_tilemap[0]->set_scrollx(0, tharrier_scroll);

	bg_update(screen, bitmap, cliprect, 0);
	tx_update(screen, bitmap, cliprect);
	draw_sprites(screen, bitmap, cliprect, m_spriteram_old2.get());
	return 0;
}

u32 nmk16_state::screen_update_strahl(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	screen.priority().fill(0, cliprect);
	bg_update(screen, bitmap, cliprect, 0);
	bg_update(screen, bitmap, cliprect, 1);
	tx_update(screen, bitmap, cliprect);
	draw_sprites(screen, bitmap, cliprect, m_spriteram_old2.get());
	return 0;
}

u32 nmk16_state::screen_update_bjtwin(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	screen.priority().fill(0, cliprect);
	bg_update(screen, bitmap, cliprect, 0);
	draw_sprites(screen, bitmap, cliprect, m_spriteram_old.get()); // only a single buffer, verified
	return 0;
}

void nmk16_state::screen_vblank_powerins_bootleg(int state)
{
	if (state)
	{
		m_maincpu->set_input_line(4, HOLD_LINE);
		// bootlegs don't have DMA?
		memcpy(m_spriteram_old2.get(),m_spriteram_old.get(), 0x1000);
		memcpy(m_spriteram_old.get(), m_mainram + m_sprdma_base / 2, 0x1000);
	}
}


/***************************************************************************


                            Video Hardware Init


***************************************************************************/

TILE_GET_INFO_MEMBER(afega_state::get_bg_tile_info_8bit)
{
	const u16 code = m_bgvideoram[0][tile_index];
	tileinfo.set(1, code, 0, 0);
}

VIDEO_START_MEMBER(afega_state,grdnstrm)
{
	// 8bpp Tilemap
	m_bg_tilemap[0] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(afega_state::get_bg_tile_info_8bit)), tilemap_mapper_delegate(*this, FUNC(afega_state::tilemap_scan_pages)), 16, 16, 256, 32);
	m_bg_tilemap[1] = nullptr;
	m_tx_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(afega_state::common_get_tx_tile_info)), TILEMAP_SCAN_COLS, 8, 8, 32, 32);

	m_tx_tilemap->set_transparent_pen(15);

	video_init();
	m_bg_tilemap[0]->set_scrolldx(92, 92);
	m_tx_tilemap->set_scrolldx(92, 92);
}


/***************************************************************************


                                Screen Drawing


***************************************************************************/

void afega_state::video_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect,
	int dsw_flipscreen,         // 1 = Horizontal and vertical screen flip are hardwired to 2 dip switches
	int xoffset, int yoffset,   // bg_tilemap0 offsets
	int attr_mask               // "sprite active" mask
	)
{
	screen.priority().fill(0, cliprect);
	if (dsw_flipscreen)
	{
		flip_screen_x_set(~m_dsw_io[0]->read() & 0x0100);
		flip_screen_y_set(~m_dsw_io[0]->read() & 0x0200);
	}

	m_bg_tilemap[0]->set_scrollx(0, m_afega_scroll[0][1] + xoffset);
	m_bg_tilemap[0]->set_scrolly(0, m_afega_scroll[0][0] + yoffset);

	m_tx_tilemap->set_scrollx(0, m_afega_scroll[1][1]);
	m_tx_tilemap->set_scrolly(0, m_afega_scroll[1][0]);

	m_bg_tilemap[0]->draw(screen, bitmap, cliprect, 0, 1);

	m_tx_tilemap->draw(screen, bitmap, cliprect, 0, 2);

	draw_sprites(screen, bitmap, cliprect, m_spriteram_old2.get());
}

void afega_state::redhawki_video_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	screen.priority().fill(0, cliprect);
	m_bg_tilemap[0]->set_scrollx(0, m_afega_scroll[1][0]&0xff);
	m_bg_tilemap[0]->set_scrolly(0, m_afega_scroll[1][1]&0xff);

	m_bg_tilemap[0]->draw(screen, bitmap, cliprect, 0, 1);

	draw_sprites(screen, bitmap, cliprect, m_spriteram_old2.get());
}

u32 afega_state::screen_update_afega(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)   { video_update(screen, bitmap, cliprect, 1, -0x100, +0x000, 0x0001);  return 0; }
u32 afega_state::screen_update_bubl2000(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect){ video_update(screen, bitmap, cliprect, 0, -0x100, +0x000, 0x0001);  return 0; } // no flipscreen support, I really would confirmation from the schematics
u32 afega_state::screen_update_redhawkb(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect){ video_update(screen, bitmap, cliprect, 0, +0x000, +0x100, 0x0001);  return 0; }
u32 afega_state::screen_update_redhawki(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect){ redhawki_video_update(screen, bitmap, cliprect); return 0;} // strange scroll regs

u32 afega_state::screen_update_firehawk(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	screen.priority().fill(0, cliprect);
	m_bg_tilemap[0]->set_scrolly(0, m_afega_scroll[1][1] + 0x100);
	m_bg_tilemap[0]->set_scrollx(0, m_afega_scroll[0][1] - 0x100);

	m_bg_tilemap[0]->draw(screen, bitmap, cliprect, 0, 1);

	m_tx_tilemap->draw(screen, bitmap, cliprect, 0, 2);

	draw_sprites(screen, bitmap, cliprect, m_spriteram_old2.get());
	return 0;
}
