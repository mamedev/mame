// license:BSD-3-Clause
// copyright-holders:Dan Boris, Mirko Buffoni
/***************************************************************************

    Atari Cloak & Dagger hardware

***************************************************************************/

#include "emu.h"
#include "video/resnet.h"
#include "includes/cloak.h"


#define NUM_PENS    (0x40)

/***************************************************************************

  CLOAK & DAGGER uses RAM to dynamically
  create the palette. The resolution is 9 bit (3 bits per gun). The palette
  contains 64 entries, but it is accessed through a memory windows 128 bytes
  long: writing to the first 64 bytes sets the MSB of the red component to 0,
  while writing to the last 64 bytes sets it to 1.

  Colors 0-15  Character mapped graphics
  Colors 16-31 Bitmapped graphics (2 palettes selected by 128H)
  Colors 32-47 Sprites
  Colors 48-63 not used

  These are the exact resistor values from the schematics:

  bit 8 -- diode |< -- pullup 1 kohm -- 2.2 kohm resistor -- pulldown 100 pf -- RED
        -- diode |< -- pullup 1 kohm -- 4.7 kohm resistor -- pulldown 100 pf -- RED
        -- diode |< -- pullup 1 kohm -- 10  kohm resistor -- pulldown 100 pf -- RED
        -- diode |< -- pullup 1 kohm -- 2.2 kohm resistor -- pulldown 100 pf -- GREEN
        -- diode |< -- pullup 1 kohm -- 4.7 kohm resistor -- pulldown 100 pf -- GREEN
        -- diode |< -- pullup 1 kohm -- 10  kohm resistor -- pulldown 100 pf -- GREEN
        -- diode |< -- pullup 1 kohm -- 2.2 kohm resistor -- pulldown 100 pf -- BLUE
        -- diode |< -- pullup 1 kohm -- 4.7 kohm resistor -- pulldown 100 pf -- BLUE
  bit 0 -- diode |< -- pullup 1 kohm -- 10  kohm resistor -- pulldown 100 pf -- BLUE

***************************************************************************/

WRITE8_MEMBER(cloak_state::cloak_paletteram_w)
{
	m_palette_ram[offset & 0x3f] = ((offset & 0x40) << 2) | data;
	set_pen(offset & 0x3f);
}


void cloak_state::set_pen(int i)
{
	UINT16 *palette_ram = m_palette_ram;
	static const int resistances[3] = { 10000, 4700, 2200 };
	double weights[3];

	/* compute the color output resistor weights */
	compute_resistor_weights(0, 255, -1.0,
								3, resistances, weights, 0, 1000,
								0, nullptr, nullptr, 0, 0,
								0, nullptr, nullptr, 0, 0);

	int r, g, b;
	int bit0, bit1, bit2;

	/* red component */
	bit0 = (~palette_ram[i] >> 6) & 0x01;
	bit1 = (~palette_ram[i] >> 7) & 0x01;
	bit2 = (~palette_ram[i] >> 8) & 0x01;
	r = combine_3_weights(weights, bit0, bit1, bit2);

	/* green component */
	bit0 = (~palette_ram[i] >> 3) & 0x01;
	bit1 = (~palette_ram[i] >> 4) & 0x01;
	bit2 = (~palette_ram[i] >> 5) & 0x01;
	g = combine_3_weights(weights, bit0, bit1, bit2);

	/* blue component */
	bit0 = (~palette_ram[i] >> 0) & 0x01;
	bit1 = (~palette_ram[i] >> 1) & 0x01;
	bit2 = (~palette_ram[i] >> 2) & 0x01;
	b = combine_3_weights(weights, bit0, bit1, bit2);

	m_palette->set_pen_color(i, rgb_t(r, g, b));
}


void cloak_state::set_current_bitmap_videoram_pointer()
{
	m_current_bitmap_videoram_accessed  = m_bitmap_videoram_selected ? m_bitmap_videoram1 : m_bitmap_videoram2;
	m_current_bitmap_videoram_displayed = m_bitmap_videoram_selected ? m_bitmap_videoram2 : m_bitmap_videoram1;
}

WRITE8_MEMBER(cloak_state::cloak_clearbmp_w)
{
//  m_screen->update_now();
	m_screen->update_partial(m_screen->vpos());

	m_bitmap_videoram_selected = data & 0x01;
	set_current_bitmap_videoram_pointer();

	if (data & 0x02)    /* clear */
		memset(m_current_bitmap_videoram_accessed, 0, 256*256);
}

void cloak_state::adjust_xy(int offset)
{
	switch (offset)
	{
		case 0x00:  m_bitmap_videoram_address_x--; m_bitmap_videoram_address_y++; break;
		case 0x01:                       m_bitmap_videoram_address_y--; break;
		case 0x02:  m_bitmap_videoram_address_x--;                            break;
		case 0x04:  m_bitmap_videoram_address_x++; m_bitmap_videoram_address_y++; break;
		case 0x05:                       m_bitmap_videoram_address_y++; break;
		case 0x06:  m_bitmap_videoram_address_x++;                            break;
	}
}

READ8_MEMBER(cloak_state::graph_processor_r)
{
	UINT8 ret = m_current_bitmap_videoram_displayed[(m_bitmap_videoram_address_y << 8) | m_bitmap_videoram_address_x];

	adjust_xy(offset);

	return ret;
}

WRITE8_MEMBER(cloak_state::graph_processor_w)
{
	switch (offset)
	{
		case 0x03: m_bitmap_videoram_address_x = data; break;
		case 0x07: m_bitmap_videoram_address_y = data; break;
		default:
			m_current_bitmap_videoram_accessed[(m_bitmap_videoram_address_y << 8) | m_bitmap_videoram_address_x] = data & 0x0f;

			adjust_xy(offset);
			break;
	}
}

WRITE8_MEMBER(cloak_state::cloak_videoram_w)
{
	UINT8 *videoram = m_videoram;

	videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}

WRITE8_MEMBER(cloak_state::cloak_flipscreen_w)
{
	flip_screen_set(data & 0x80);
}

TILE_GET_INFO_MEMBER(cloak_state::get_bg_tile_info)
{
	UINT8 *videoram = m_videoram;
	int code = videoram[tile_index];

	SET_TILE_INFO_MEMBER(0, code, 0, 0);
}

void cloak_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(cloak_state::get_bg_tile_info),this), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);

	m_bitmap_videoram1 = auto_alloc_array(machine(), UINT8, 256*256);
	m_bitmap_videoram2 = auto_alloc_array(machine(), UINT8, 256*256);
	m_palette_ram = auto_alloc_array(machine(), UINT16, NUM_PENS);

	set_current_bitmap_videoram_pointer();

	save_item(NAME(m_bitmap_videoram_address_x));
	save_item(NAME(m_bitmap_videoram_address_y));
	save_item(NAME(m_bitmap_videoram_selected));
	save_pointer(NAME(m_bitmap_videoram1), 256*256);
	save_pointer(NAME(m_bitmap_videoram2), 256*256);
	save_pointer(NAME(m_palette_ram), NUM_PENS);
	machine().save().register_postload(save_prepost_delegate(FUNC(cloak_state::set_current_bitmap_videoram_pointer), this));
}

void cloak_state::draw_bitmap(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int x, y;

	for (y = cliprect.min_y; y <= cliprect.max_y; y++)
		for (x = cliprect.min_x; x <= cliprect.max_x; x++)
		{
			pen_t pen = m_current_bitmap_videoram_displayed[(y << 8) | x] & 0x07;

			if (pen)
				bitmap.pix16(y, (x - 6) & 0xff) = 0x10 | ((x & 0x80) >> 4) | pen;
		}
}

void cloak_state::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	UINT8 *spriteram = m_spriteram;
	int offs;

	for (offs = (0x100 / 4) - 1; offs >= 0; offs--)
	{
		int code = spriteram[offs + 64] & 0x7f;
		int flipx = spriteram[offs + 64] & 0x80;
		int flipy = 0;
		int sx = spriteram[offs + 192];
		int sy = 240 - spriteram[offs];

		if (flip_screen())
		{
			sx -= 9;
			sy = 240 - sy;
			flipx = !flipx;
			flipy = !flipy;
		}

		m_gfxdecode->gfx(1)->transpen(bitmap,cliprect, code, 0, flipx, flipy,   sx, sy, 0);
	}
}

UINT32 cloak_state::screen_update_cloak(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	draw_bitmap(bitmap, cliprect);
	draw_sprites(bitmap, cliprect);
	return 0;
}
