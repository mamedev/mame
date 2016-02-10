// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
#include "emu.h"
#include "includes/deniam.h"


void deniam_state::deniam_common_init(  )
{
	int i;

	m_bg_scrollx_reg = 0x00a4/2;
	m_bg_scrolly_reg = 0x00a8/2;
	m_bg_page_reg    = 0x00ac/2;
	m_fg_scrollx_reg = 0x00a2/2;
	m_fg_scrolly_reg = 0x00a6/2;
	m_fg_page_reg    = 0x00aa/2;

	m_display_enable = 0;
	m_coinctrl = 0;

	for (i = 0; i < 4; i++)
	{
		m_bg_page[i] = 0;
		m_fg_page[i] = 0;
	}
}

DRIVER_INIT_MEMBER(deniam_state,logicpro)
{
	deniam_common_init();

	m_bg_scrollx_offs = 0x00d;
	m_bg_scrolly_offs = 0x000;
	m_fg_scrollx_offs = 0x009;
	m_fg_scrolly_offs = 0x000;
}

DRIVER_INIT_MEMBER(deniam_state,karianx)
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
	int page = tile_index >> 11;
	UINT16 attr = m_videoram[m_bg_page[page] * 0x0800 + (tile_index & 0x7ff)];
	SET_TILE_INFO_MEMBER(0,
			attr,
			(attr & 0x1fc0) >> 6,
			0);
}

TILE_GET_INFO_MEMBER(deniam_state::get_fg_tile_info)
{
	int page = tile_index >> 11;
	UINT16 attr = m_videoram[m_fg_page[page] * 0x0800 + (tile_index & 0x7ff)];
	SET_TILE_INFO_MEMBER(0,
			attr,
			(attr & 0x1fc0) >> 6,
			0);
}

TILE_GET_INFO_MEMBER(deniam_state::get_tx_tile_info)
{
	UINT16 attr = m_textram[tile_index];
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
	m_bg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(deniam_state::get_bg_tile_info),this), tilemap_mapper_delegate(FUNC(deniam_state::scan_pages),this), 8, 8, 128, 64);
	m_fg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(deniam_state::get_fg_tile_info),this), tilemap_mapper_delegate(FUNC(deniam_state::scan_pages),this), 8, 8, 128, 64);
	m_tx_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(deniam_state::get_tx_tile_info),this), TILEMAP_SCAN_ROWS, 8, 8, 64, 32);

	m_fg_tilemap->set_transparent_pen(0);
	m_tx_tilemap->set_transparent_pen(0);
}



/***************************************************************************

  Memory handlers

***************************************************************************/

WRITE16_MEMBER(deniam_state::deniam_videoram_w)
{
	int page, i;
	COMBINE_DATA(&m_videoram[offset]);

	page = offset >> 11;
	for (i = 0; i < 4; i++)
	{
		if (m_bg_page[i] == page)
			m_bg_tilemap->mark_tile_dirty(i * 0x800 + (offset & 0x7ff));
		if (m_fg_page[i] == page)
			m_fg_tilemap->mark_tile_dirty(i * 0x800 + (offset & 0x7ff));
	}
}


WRITE16_MEMBER(deniam_state::deniam_textram_w)
{
	COMBINE_DATA(&m_textram[offset]);
	m_tx_tilemap->mark_tile_dirty(offset);
}


WRITE16_MEMBER(deniam_state::deniam_palette_w)
{
	int r, g, b;

	data = COMBINE_DATA(&m_paletteram[offset]);

	r = ((data << 1) & 0x1e) | ((data >> 12) & 0x01);
	g = ((data >> 3) & 0x1e) | ((data >> 13) & 0x01);
	b = ((data >> 7) & 0x1e) | ((data >> 14) & 0x01);
	m_palette->set_pen_color(offset, pal5bit(r), pal5bit(g), pal5bit(b));
}

READ16_MEMBER(deniam_state::deniam_coinctrl_r)
{
	return m_coinctrl;
}

WRITE16_MEMBER(deniam_state::deniam_coinctrl_w)
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
	int offs;
	UINT8 *gfx = memregion("gfx2")->base();

	for (offs = m_spriteram.bytes() / 2 - 8; offs >= 0; offs -= 8)
	{
		int sx, starty, endy, x, y, start, color, width, flipx, primask;
		UINT8 *rom = gfx;

		sx = (m_spriteram[offs + 1] & 0x01ff) + 16 * 8 - 1;
		if (sx >= 512) sx -= 512;
		starty = m_spriteram[offs + 0] & 0xff;
		endy = m_spriteram[offs + 0] >> 8;

		width = m_spriteram[offs + 2] & 0x007f;
		flipx = m_spriteram[offs + 2] & 0x0100;
		if (flipx) sx++;

		color = 0x40 + (m_spriteram[offs + 4] & 0x3f);

		primask = 8;
		switch (m_spriteram[offs + 4] & 0xc0)
		{
			case 0x00: primask |= 4 | 2 | 1; break; /* below everything */
			case 0x40: primask |= 4 | 2;     break; /* below fg and tx */
			case 0x80: primask |= 4;         break; /* below tx */
			case 0xc0:                       break; /* above everything */
		}


		start = m_spriteram[offs + 3] + ((m_spriteram[offs + 4] & 0x1f00) << 8);
		rom += 2 * start;

		for (y = starty + 1; y <= endy; y++)
		{
			int drawing = 0;
			int i = 0;

			rom += 2 * width;   /* note that the first line is skipped */
			x = 0;
			while (i < 512) /* safety check */
			{
				if (flipx)
				{
					if ((rom[i] & 0x0f) == 0x0f)
					{
						if (!drawing) drawing = 1;
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
						if (!drawing) drawing = 1;
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
						if (!drawing) drawing = 1;
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
						if (!drawing) drawing = 1;
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
	int tile_index;

	if (m_bg_page[page] != value)
	{
		m_bg_page[page] = value;
		for (tile_index = page * 0x800; tile_index < (page + 1) * 0x800; tile_index++)
			m_bg_tilemap->mark_tile_dirty(tile_index);
	}
}

void deniam_state::set_fg_page( int page, int value )
{
	int tile_index;

	if (m_fg_page[page] != value)
	{
		m_fg_page[page] = value;
		for (tile_index = page * 0x800; tile_index < (page + 1) * 0x800; tile_index++)
			m_fg_tilemap->mark_tile_dirty(tile_index);
	}
}

UINT32 deniam_state::screen_update_deniam(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int bg_scrollx, bg_scrolly, fg_scrollx, fg_scrolly;
	int page;

	if (!m_display_enable)
		return 0;   /* don't update (freeze display) */

	bg_scrollx = m_textram[m_bg_scrollx_reg] - m_bg_scrollx_offs;
	bg_scrolly = (m_textram[m_bg_scrolly_reg] & 0xff) - m_bg_scrolly_offs;
	page = m_textram[m_bg_page_reg];
	set_bg_page(3, (page >>12) & 0x0f);
	set_bg_page(2, (page >> 8) & 0x0f);
	set_bg_page(1, (page >> 4) & 0x0f);
	set_bg_page(0, (page >> 0) & 0x0f);

	fg_scrollx = m_textram[m_fg_scrollx_reg] - m_fg_scrollx_offs;
	fg_scrolly = (m_textram[m_fg_scrolly_reg] & 0xff) - m_fg_scrolly_offs;
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
