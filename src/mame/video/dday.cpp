// license:BSD-3-Clause
// copyright-holders:Zsolt Vasvari
/***************************************************************************

  video.c

  Functions to emulate the video hardware of the machine.

  Convention: "sl" stands for "searchlight"

***************************************************************************/

#include "emu.h"
#include "sound/ay8910.h"
#include "includes/dday.h"


/* Note: There seems to be no way to reset this timer via hardware.
         The game uses a difference method to reset it to 99.

  Thanks Zwaxy for the timer info. */

READ8_MEMBER(dday_state::dday_countdown_timer_r)
{
	return ((m_timer_value / 10) << 4) | (m_timer_value % 10);
}

TIMER_CALLBACK_MEMBER(dday_state::countdown_timer_callback)
{
	m_timer_value--;

	if (m_timer_value < 0)
		m_timer_value = 99;
}

void dday_state::start_countdown_timer()
{
	m_timer_value = 0;

	machine().scheduler().timer_pulse(attotime::from_seconds(1), timer_expired_delegate(FUNC(dday_state::countdown_timer_callback),this));
}


/***************************************************************************

  Convert the color PROMs into a more useable format.

***************************************************************************/

PALETTE_INIT_MEMBER(dday_state, dday)
{
	const UINT8 *color_prom = memregion("proms")->base();
	int i;

	palette.set_shadow_factor(1.0 / 8);

	/* create a lookup table for the palette */
	for (i = 0; i < 0x100; i++)
	{
		int r = pal4bit(color_prom[i + 0x000]);
		int g = pal4bit(color_prom[i + 0x100]);
		int b = pal4bit(color_prom[i + 0x200]);

		palette.set_indirect_color(i, rgb_t(r, g, b));
	}

	for (i = 0; i < 0x100; i++)
		palette.set_pen_indirect(i, i);

	/* HACK!!! This table is handgenerated, but it matches the screenshot.
	   I have no clue how it really works */
	palette.set_pen_indirect(0*8+0+0, 0x00);
	palette.set_pen_indirect(0*8+0+1, 0x01);
	palette.set_pen_indirect(0*8+0+2, 0x15);
	palette.set_pen_indirect(0*8+0+3, 0x02);
	palette.set_pen_indirect(0*8+4+0, 0x00);
	palette.set_pen_indirect(0*8+4+1, 0x01);
	palette.set_pen_indirect(0*8+4+2, 0x15);
	palette.set_pen_indirect(0*8+4+3, 0x02);

	palette.set_pen_indirect(1*8+0+0, 0x04);
	palette.set_pen_indirect(1*8+0+1, 0x05);
	palette.set_pen_indirect(1*8+0+2, 0x03);
	palette.set_pen_indirect(1*8+0+3, 0x07);
	palette.set_pen_indirect(1*8+4+0, 0x04);
	palette.set_pen_indirect(1*8+4+1, 0x05);
	palette.set_pen_indirect(1*8+4+2, 0x03);
	palette.set_pen_indirect(1*8+4+3, 0x07);

	palette.set_pen_indirect(2*8+0+0, 0x08);
	palette.set_pen_indirect(2*8+0+1, 0x15);
	palette.set_pen_indirect(2*8+0+2, 0x0a);
	palette.set_pen_indirect(2*8+0+3, 0x03);
	palette.set_pen_indirect(2*8+4+0, 0x08);
	palette.set_pen_indirect(2*8+4+1, 0x15);
	palette.set_pen_indirect(2*8+4+2, 0x0a);
	palette.set_pen_indirect(2*8+4+3, 0x03);

	palette.set_pen_indirect(3*8+0+0, 0x08);
	palette.set_pen_indirect(3*8+0+1, 0x15);
	palette.set_pen_indirect(3*8+0+2, 0x0a);
	palette.set_pen_indirect(3*8+0+3, 0x03);
	palette.set_pen_indirect(3*8+4+0, 0x08);
	palette.set_pen_indirect(3*8+4+1, 0x15);
	palette.set_pen_indirect(3*8+4+2, 0x0a);
	palette.set_pen_indirect(3*8+4+3, 0x03);

	palette.set_pen_indirect(4*8+0+0, 0x10);
	palette.set_pen_indirect(4*8+0+1, 0x11);
	palette.set_pen_indirect(4*8+0+2, 0x12);
	palette.set_pen_indirect(4*8+0+3, 0x07);
	palette.set_pen_indirect(4*8+4+0, 0x10);
	palette.set_pen_indirect(4*8+4+1, 0x11);
	palette.set_pen_indirect(4*8+4+2, 0x12);
	palette.set_pen_indirect(4*8+4+3, 0x07);

	palette.set_pen_indirect(5*8+0+0, 0x1d);
	palette.set_pen_indirect(5*8+0+1, 0x15);
	palette.set_pen_indirect(5*8+0+2, 0x16);
	palette.set_pen_indirect(5*8+0+3, 0x1b);
	palette.set_pen_indirect(5*8+4+0, 0x1d);
	palette.set_pen_indirect(5*8+4+1, 0x15);
	palette.set_pen_indirect(5*8+4+2, 0x16);
	palette.set_pen_indirect(5*8+4+3, 0x1b);

	palette.set_pen_indirect(6*8+0+0, 0x1d);
	palette.set_pen_indirect(6*8+0+1, 0x15);
	palette.set_pen_indirect(6*8+0+2, 0x1a);
	palette.set_pen_indirect(6*8+0+3, 0x1b);
	palette.set_pen_indirect(6*8+4+0, 0x1d);
	palette.set_pen_indirect(6*8+4+1, 0x15);
	palette.set_pen_indirect(6*8+4+2, 0x1a);
	palette.set_pen_indirect(6*8+4+3, 0x1b);

	palette.set_pen_indirect(7*8+0+0, 0x1d);
	palette.set_pen_indirect(7*8+0+1, 0x02);
	palette.set_pen_indirect(7*8+0+2, 0x04);
	palette.set_pen_indirect(7*8+0+3, 0x1b);
	palette.set_pen_indirect(7*8+4+0, 0x1d);
	palette.set_pen_indirect(7*8+4+1, 0x02);
	palette.set_pen_indirect(7*8+4+2, 0x04);
	palette.set_pen_indirect(7*8+4+3, 0x1b);
}


/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

TILE_GET_INFO_MEMBER(dday_state::get_bg_tile_info)
{
	int code;

	code = m_bgvideoram[tile_index];
	SET_TILE_INFO_MEMBER(0, code, code >> 5, 0);
}

TILE_GET_INFO_MEMBER(dday_state::get_fg_tile_info)
{
	int code, flipx;

	flipx = m_colorram[tile_index & 0x03e0] & 0x01;
	code = m_fgvideoram[flipx ? tile_index ^ 0x1f : tile_index];
	SET_TILE_INFO_MEMBER(2, code, code >> 5, flipx ? TILE_FLIPX : 0);
}

TILE_GET_INFO_MEMBER(dday_state::get_text_tile_info)
{
	int code;

	code = m_textvideoram[tile_index];
	SET_TILE_INFO_MEMBER(1, code, code >> 5, 0);
}

TILE_GET_INFO_MEMBER(dday_state::get_sl_tile_info)
{
	int code, sl_flipx, flipx;
	UINT8* sl_map;

	sl_map = &memregion("user1")->base()[(m_sl_image & 0x07) * 0x0200];

	flipx = (tile_index >> 4) & 0x01;
	sl_flipx = (m_sl_image >> 3) & 0x01;

	/* bit 4 is really a flip indicator.  Need to shift bits 5-9 to the right by 1 */
	tile_index = ((tile_index & 0x03e0) >> 1) | (tile_index & 0x0f);

	code = sl_map[flipx ? tile_index ^ 0x0f : tile_index];

	if ((sl_flipx != flipx) && (code & 0x80))
		/* no mirroring, draw dark spot */
		code = 1;

	SET_TILE_INFO_MEMBER(3, code & 0x3f, 0, flipx ? TILE_FLIPX : 0);
}


/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

void dday_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(dday_state::get_bg_tile_info),this), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
	m_fg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(dday_state::get_fg_tile_info),this), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
	m_text_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(dday_state::get_text_tile_info),this), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
	m_sl_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(dday_state::get_sl_tile_info),this), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);

	m_screen->register_screen_bitmap(m_main_bitmap);

	m_bg_tilemap->set_transmask(0, 0x00f0, 0xff0f); /* pens 0-3 have priority over the foreground layer */
	m_fg_tilemap->set_transparent_pen(0);
	m_text_tilemap->set_transparent_pen(0);

	start_countdown_timer();
}

WRITE8_MEMBER(dday_state::dday_bgvideoram_w)
{
	m_bgvideoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}

WRITE8_MEMBER(dday_state::dday_fgvideoram_w)
{
	m_fgvideoram[offset] = data;
	m_fg_tilemap->mark_tile_dirty(offset);
	m_fg_tilemap->mark_tile_dirty(offset ^ 0x1f);  /* for flipx case */
}

WRITE8_MEMBER(dday_state::dday_textvideoram_w)
{
	m_textvideoram[offset] = data;
	m_text_tilemap->mark_tile_dirty(offset);
}

WRITE8_MEMBER(dday_state::dday_colorram_w)
{
	int i;

	offset &= 0x03e0;

	m_colorram[offset & 0x3e0] = data;

	for (i = 0; i < 0x20; i++)
		m_fg_tilemap->mark_tile_dirty(offset + i);
}

READ8_MEMBER(dday_state::dday_colorram_r)
{
	return m_colorram[offset & 0x03e0];
}


WRITE8_MEMBER(dday_state::dday_sl_control_w)
{
	if (m_sl_image != data)
	{
		m_sl_image = data;
		m_sl_tilemap->mark_all_dirty();
	}
}


WRITE8_MEMBER(dday_state::dday_control_w)
{
	//if (data & 0xac)  logerror("Control = %02X\n", data & 0xac);

	/* bit 0 is coin counter 1 */
	machine().bookkeeping().coin_counter_w(0, data & 0x01);

	/* bit 1 is coin counter 2 */
	machine().bookkeeping().coin_counter_w(1, data & 0x02);

	/* bit 4 is sound enable */
	if (!(data & 0x10) && (m_control & 0x10))
		m_ay1->reset();

	machine().sound().system_enable(data & 0x10);

	/* bit 6 is search light enable */
	m_sl_enable = data & 0x40;

	m_control = data;
}

/***************************************************************************

  Display refresh

***************************************************************************/

UINT32 dday_state::screen_update_dday(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->draw(screen, m_main_bitmap, cliprect, TILEMAP_DRAW_LAYER1, 0);
	m_fg_tilemap->draw(screen, m_main_bitmap, cliprect, 0, 0);
	m_bg_tilemap->draw(screen, m_main_bitmap, cliprect, TILEMAP_DRAW_LAYER0, 0);
	m_text_tilemap->draw(screen, m_main_bitmap, cliprect, 0, 0);

	if (m_sl_enable)
	{
		/* apply shadow */
		int x, y;

		bitmap_ind16 &sl_bitmap = m_sl_tilemap->pixmap();

		for (x = cliprect.min_x; x <= cliprect.max_x; x++)
			for (y = cliprect.min_y; y <= cliprect.max_y; y++)
			{
				UINT16 src_pixel = m_main_bitmap.pix16(y, x);

				if (sl_bitmap.pix16(y, x) == 0xff)
					src_pixel += m_palette->entries();

				bitmap.pix16(y, x) = src_pixel;
			}
	}
	else
		copybitmap(bitmap, m_main_bitmap, 0, 0, 0, 0, cliprect);

	return 0;
}
