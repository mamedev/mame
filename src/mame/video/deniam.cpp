// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
#include "emu.h"
#include "includes/deniam.h"
#include "screen.h"


void deniam_state::deniam_common_init(  )
{
	m_bg_scrollx_reg = 0x00a4/2;
	m_bg_scrolly_reg = 0x00a8/2;
	m_bg_page_reg    = 0x00ac/2;
	m_fg_scrollx_reg = 0x00a2/2;
	m_fg_scrolly_reg = 0x00a6/2;
	m_fg_page_reg    = 0x00aa/2;

	m_display_enable = 0;
	m_coinctrl = 0;

	for (int i = 0; i < 4; i++)
	{
		m_bg_page[i] = 0;
		m_fg_page[i] = 0;
	}
}

void deniam_state::init_logicpro()
{
	deniam_common_init();

	m_bg_scrollx_offs = 0x00d;
	m_bg_scrolly_offs = 0x000;
	m_fg_scrollx_offs = 0x009;
	m_fg_scrolly_offs = 0x000;
}

void deniam_state::init_karianx()
{
	deniam_common_init();

	m_bg_scrollx_offs = 0x10d;
	m_bg_scrolly_offs = 0x080;
	m_fg_scrollx_offs = 0x109;
	m_fg_scrolly_offs = 0x080;
}



/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

TILEMAP_MAPPER_MEMBER(deniam_state::scan_pages)
{
	/* logical (col,row) -> memory offset */
	return (col & 0x3f) + ((row & 0x1f) << 6) + ((col & 0x40) << 5) + ((row & 0x20) << 7);
}

TILE_GET_INFO_MEMBER(deniam_state::get_bg_tile_info)
{
	const int page = tile_index >> 11;
	u16 attr = m_videoram[m_bg_page[page] * 0x0800 + (tile_index & 0x7ff)];
	SET_TILE_INFO_MEMBER(0,
			attr,
			(attr & 0x1fc0) >> 6,
			0);
}

TILE_GET_INFO_MEMBER(deniam_state::get_fg_tile_info)
{
	const int page = tile_index >> 11;
	u16 attr = m_videoram[m_fg_page[page] * 0x0800 + (tile_index & 0x7ff)];
	SET_TILE_INFO_MEMBER(0,
			attr,
			(attr & 0x1fc0) >> 6,
			0);
}

TILE_GET_INFO_MEMBER(deniam_state::get_tx_tile_info)
{
	u16 attr = m_textram[tile_index];
	SET_TILE_INFO_MEMBER(0,
			attr & 0xf1ff,
			(attr & 0x0e00) >> 9,
			0);
}



/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

void deniam_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(FUNC(deniam_state::get_bg_tile_info),this), tilemap_mapper_delegate(FUNC(deniam_state::scan_pages),this), 8, 8, 128, 64);
	m_fg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(FUNC(deniam_state::get_fg_tile_info),this), tilemap_mapper_delegate(FUNC(deniam_state::scan_pages),this), 8, 8, 128, 64);
	m_tx_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(FUNC(deniam_state::get_tx_tile_info),this), TILEMAP_SCAN_ROWS, 8, 8, 64, 32);

	m_fg_tilemap->set_transparent_pen(0);
	m_tx_tilemap->set_transparent_pen(0);
}



/***************************************************************************

  Memory handlers

***************************************************************************/

void deniam_state::videoram_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_videoram[offset]);

	const int page = offset >> 11;
	for (int i = 0; i < 4; i++)
	{
		if (m_bg_page[i] == page)
			m_bg_tilemap->mark_tile_dirty(i * 0x800 + (offset & 0x7ff));
		if (m_fg_page[i] == page)
			m_fg_tilemap->mark_tile_dirty(i * 0x800 + (offset & 0x7ff));
	}
}


void deniam_state::textram_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_textram[offset]);
	m_tx_tilemap->mark_tile_dirty(offset);
}


u16 deniam_state::coinctrl_r()
{
	return m_coinctrl;
}

void deniam_state::coinctrl_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_coinctrl);

	/* bit 0 is coin counter */
	machine().bookkeeping().coin_counter_w(0, m_coinctrl & 0x01);

	/* bit 6 is display enable (0 freezes screen) */
	m_display_enable = m_coinctrl & 0x20;

	/* other bits unknown (unused?) */
}



/***************************************************************************

  Display refresh

***************************************************************************/

/*
 * Sprite Format
 * ------------------
 *
 * Word | Bit(s)           | Use
 * -----+-fedcba9876543210-+----------------
 *   0  | --------xxxxxxxx | display y start
 *   0  | xxxxxxxx-------- | display y end
 *   2  | -------xxxxxxxxx | x position
 *   2  | ------x--------- | unknown (used in logicpr2, maybe just a bug?)
 *   2  | xxxxxx---------- | unused?
 *   4  | ---------xxxxxxx | width
 *   4  | --------x------- | is this flip y like in System 16?
 *   4  | -------x-------- | flip x
 *   4  | xxxxxxx--------- | unused?
 *   6  | xxxxxxxxxxxxxxxx | ROM address low bits
 *   8  | ----------xxxxxx | color
 *   8  | --------xx------ | priority
 *   8  | ---xxxxx-------- | ROM address high bits
 *   8  | xxx------------- | unused? (extra address bits for larger ROMs?)
 *   a  | ---------------- | zoomx like in System 16?
 *   c  | ---------------- | zoomy like in System 16?
 *   e  | ---------------- |
 */
void deniam_state::draw_sprites( screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	for (int offs = m_spriteram.bytes() / 2 - 8; offs >= 0; offs -= 8)
	{
		u8 *rom = m_spritegfx->base();

		int sx = (m_spriteram[offs + 1] & 0x01ff) + 16 * 8 - 1;
		if (sx >= 512) sx -= 512;
		const int starty = m_spriteram[offs + 0] & 0xff;
		const int endy = m_spriteram[offs + 0] >> 8;

		const int width = m_spriteram[offs + 2] & 0x007f;
		bool flipx = m_spriteram[offs + 2] & 0x0100;
		if (flipx) sx++;

		const u32 color = 0x40 + (m_spriteram[offs + 4] & 0x3f);

		int primask = 8;
		switch (m_spriteram[offs + 4] & 0xc0)
		{
			case 0x00: primask |= 4 | 2 | 1; break; /* below everything */
			case 0x40: primask |= 4 | 2;     break; /* below fg and tx */
			case 0x80: primask |= 4;         break; /* below tx */
			case 0xc0:                       break; /* above everything */
		}

		const int start = m_spriteram[offs + 3] + ((m_spriteram[offs + 4] & 0x1f00) << 8);
		rom += 2 * start;

		for (int y = starty + 1; y <= endy; y++)
		{
			bool drawing = false;
			int i = 0;

			rom += 2 * width;   /* note that the first line is skipped */
			int x = 0;
			while (i < 512) /* safety check */
			{
				if (flipx)
				{
					if ((rom[i] & 0x0f) == 0x0f)
					{
						if (!drawing) drawing = true;
						else break;
					}
					else
					{
						if (rom[i] & 0x0f)
						{
							if (cliprect.contains(sx + x, y))
							{
								if ((screen.priority().pix8(y, sx + x) & primask) == 0)
									bitmap.pix16(y, sx + x) = color * 16 + (rom[i] & 0x0f);
								screen.priority().pix8(y, sx + x) = 8;
							}
						}
						x++;
					}

					if ((rom[i] & 0xf0) == 0xf0)
					{
						if (!drawing) drawing = true;
						else break;
					}
					else
					{
						if (rom[i] & 0xf0)
						{
							if (cliprect.contains(sx + x, y))
							{
								if ((screen.priority().pix8(y, sx + x) & primask) == 0)
									bitmap.pix16(y, sx + x) = color * 16+(rom[i] >> 4);
								screen.priority().pix8(y, sx + x) = 8;
							}
						}
						x++;
					}

					i--;
				}
				else
				{
					if ((rom[i] & 0xf0) == 0xf0)
					{
						if (!drawing) drawing = true;
						else break;
					}
					else
					{
						if (rom[i] & 0xf0)
						{
							if (cliprect.contains(sx + x, y))
							{
								if ((screen.priority().pix8(y, sx + x) & primask) == 0)
									bitmap.pix16(y, sx + x) = color * 16 + (rom[i] >> 4);
								screen.priority().pix8(y, sx + x) = 8;
							}
						}
						x++;
					}

					if ((rom[i] & 0x0f) == 0x0f)
					{
						if (!drawing) drawing = true;
						else break;
					}
					else
					{
						if (rom[i] & 0x0f)
						{
							if (cliprect.contains(sx + x, y))
							{
								if ((screen.priority().pix8(y, sx + x) & primask) == 0)
									bitmap.pix16(y, sx + x) = color * 16 + (rom[i] & 0x0f);
								screen.priority().pix8(y, sx + x) = 8;
							}
						}
						x++;
					}

					i++;
				}
			}
		}
	}
}

void deniam_state::set_bg_page( int page, int value )
{
	if (m_bg_page[page] != value)
	{
		m_bg_page[page] = value;
		for (int tile_index = page * 0x800; tile_index < (page + 1) * 0x800; tile_index++)
			m_bg_tilemap->mark_tile_dirty(tile_index);
	}
}

void deniam_state::set_fg_page( int page, int value )
{
	if (m_fg_page[page] != value)
	{
		m_fg_page[page] = value;
		for (int tile_index = page * 0x800; tile_index < (page + 1) * 0x800; tile_index++)
			m_fg_tilemap->mark_tile_dirty(tile_index);
	}
}

u32 deniam_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int page;

	if (!m_display_enable)
	{
		bitmap.fill(m_palette->black_pen(), cliprect);
		return 0;   /* don't update (freeze display) */
	}

	int bg_scrollx = m_textram[m_bg_scrollx_reg] - m_bg_scrollx_offs;
	int bg_scrolly = (m_textram[m_bg_scrolly_reg] & 0xff) - m_bg_scrolly_offs;
	page = m_textram[m_bg_page_reg];
	set_bg_page(3, (page >>12) & 0x0f);
	set_bg_page(2, (page >> 8) & 0x0f);
	set_bg_page(1, (page >> 4) & 0x0f);
	set_bg_page(0, (page >> 0) & 0x0f);

	int fg_scrollx = m_textram[m_fg_scrollx_reg] - m_fg_scrollx_offs;
	int fg_scrolly = (m_textram[m_fg_scrolly_reg] & 0xff) - m_fg_scrolly_offs;
	page = m_textram[m_fg_page_reg];
	set_fg_page(3, (page >>12) & 0x0f);
	set_fg_page(2, (page >> 8) & 0x0f);
	set_fg_page(1, (page >> 4) & 0x0f);
	set_fg_page(0, (page >> 0) & 0x0f);

	m_bg_tilemap->set_scrollx(0, bg_scrollx & 0x1ff);
	m_bg_tilemap->set_scrolly(0, bg_scrolly & 0x0ff);
	m_fg_tilemap->set_scrollx(0, fg_scrollx & 0x1ff);
	m_fg_tilemap->set_scrolly(0, fg_scrolly & 0x0ff);

	screen.priority().fill(0, cliprect);

	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 1);
	m_fg_tilemap->draw(screen, bitmap, cliprect, 0, 2);
	m_tx_tilemap->draw(screen, bitmap, cliprect, 0, 4);

	draw_sprites(screen, bitmap, cliprect);
	return 0;
}
