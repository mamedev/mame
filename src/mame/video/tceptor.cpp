// license:BSD-3-Clause
// copyright-holders:BUT
/*
 *  Thunder Ceptor board
 *  emulate video hardware
 */

#include "emu.h"
#include "includes/tceptor.h"


#define TX_TILE_OFFSET_CENTER   (32 * 2)
#define TX_TILE_OFFSET_RIGHT    (32 * 0 + 2)
#define TX_TILE_OFFSET_LEFT (32 * 31 + 2)

#define SPR_TRANS_COLOR     (0xff + 0x300)
#define SPR_MASK_COLOR      (0xfe + 0x300)


/*******************************************************************/

PALETTE_INIT_MEMBER(tceptor_state, tceptor)
{
	const UINT8 *color_prom = memregion("proms")->base();
	int i;

	/* create a lookup table for the palette */
	for (i = 0; i < 0x400; i++)
	{
		int r = pal4bit(color_prom[i + 0x000]);
		int g = pal4bit(color_prom[i + 0x400]);
		int b = pal4bit(color_prom[i + 0x800]);

		palette.set_indirect_color(i, rgb_t(r, g, b));
	}

	/* color_prom now points to the beginning of the lookup table */
	color_prom += 0xc00;


	/*
	      color lookup table:
	        0-    +1024 ( 4 * 256) colors: text   (use 0-   256 colors)
	        1024- +1024 (16 *  64) colors: sprite (use 768- 256 colors)
	        2048-  +512 ( 8 *  64) colors: bg     (use 0-   512 colors)
	        3840-  +256 ( 4 *  64) colors: road   (use 512- 256 colors)
	    */

	/* tiles lookup table (1024 colors) */
	for (i = 0; i < 0x0400; i++)
	{
		int ctabentry = color_prom[i];
		palette.set_pen_indirect(i, ctabentry);
	}

	/* sprites lookup table (1024 colors) */
	for (i = 0x0400; i < 0x0800; i++)
	{
		int ctabentry = color_prom[i] | 0x300;
		palette.set_pen_indirect(i, ctabentry);
	}

	/* background: no lookup PROM, use directly (512 colors) */
	for (i = 0x0a00; i < 0x0c00; i++)
	{
		int ctabentry = i & 0x1ff;
		palette.set_pen_indirect(i, ctabentry);
	}

	/* road lookup table (256 colors) */
	for (i = 0x0f00; i < 0x1000; i++)
	{
		int ctabentry = color_prom[i - 0x700] | 0x200;
		palette.set_pen_indirect(i, ctabentry);
	}

	/* setup sprite mask color map */
	/* tceptor2: only 0x23 */
	memset(m_is_mask_spr, 0, sizeof m_is_mask_spr);
	for (i = 0; i < 0x400; i++)
		if (palette.pen_indirect(i | 0x400) == SPR_MASK_COLOR)
			m_is_mask_spr[i >> 4] = 1;
}


/*******************************************************************/

inline int tceptor_state::get_tile_addr(int tile_index)
{
	int x = tile_index / 28;
	int y = tile_index % 28;

	switch (x)
	{
	case 0:
		return TX_TILE_OFFSET_LEFT + y;
	case 33:
		return TX_TILE_OFFSET_RIGHT + y;
	}

	return TX_TILE_OFFSET_CENTER + (x - 1) + y * 32;
}

TILE_GET_INFO_MEMBER(tceptor_state::get_tx_tile_info)
{
	int offset = get_tile_addr(tile_index);
	int code = m_tile_ram[offset];
	int color = m_tile_attr[offset];

	tileinfo.group = color;

	SET_TILE_INFO_MEMBER(0, code, color, 0);
}

void tceptor_state::tile_mark_dirty(int offset)
{
	int x = -1;
	int y = -1;

	if (offset >= TX_TILE_OFFSET_LEFT && offset < TX_TILE_OFFSET_LEFT + 28)
	{
		x = 0;
		y = offset - TX_TILE_OFFSET_LEFT;
	}
	else if (offset >= TX_TILE_OFFSET_RIGHT && offset < TX_TILE_OFFSET_RIGHT + 28)
	{
		x = 33;
		y = offset - TX_TILE_OFFSET_RIGHT;
	}
	else if (offset >= TX_TILE_OFFSET_CENTER && offset < TX_TILE_OFFSET_CENTER + 32 * 28)
	{
		offset -= TX_TILE_OFFSET_CENTER;
		x = (offset % 32) + 1;
		y = offset / 32;
	}

	if (x >= 0)
		m_tx_tilemap->mark_tile_dirty(x * 28 + y);
}


WRITE8_MEMBER(tceptor_state::tceptor_tile_ram_w)
{
	if (m_tile_ram[offset] != data)
	{
		m_tile_ram[offset] = data;
		tile_mark_dirty(offset);
	}
}

WRITE8_MEMBER(tceptor_state::tceptor_tile_attr_w)
{
	if (m_tile_attr[offset] != data)
	{
		m_tile_attr[offset] = data;
		tile_mark_dirty(offset);
	}
}


/*******************************************************************/

TILE_GET_INFO_MEMBER(tceptor_state::get_bg1_tile_info)
{
	UINT16 data = m_bg_ram[tile_index * 2] | (m_bg_ram[tile_index * 2 + 1] << 8);
	int code = (data & 0x3ff) | 0x000;
	int color = (data & 0xfc00) >> 10;

	SET_TILE_INFO_MEMBER(m_bg, code, color, 0);
}

TILE_GET_INFO_MEMBER(tceptor_state::get_bg2_tile_info)
{
	UINT16 data = m_bg_ram[tile_index * 2 + 0x1000] | (m_bg_ram[tile_index * 2 + 1 + 0x1000] << 8);
	int code = (data & 0x3ff) | 0x400;
	int color = (data & 0xfc00) >> 10;

	SET_TILE_INFO_MEMBER(m_bg, code, color, 0);
}

WRITE8_MEMBER(tceptor_state::tceptor_bg_ram_w)
{
	m_bg_ram[offset] = data;

	offset /= 2;
	if (offset < 0x800)
		m_bg1_tilemap->mark_tile_dirty(offset);
	else
		m_bg2_tilemap->mark_tile_dirty(offset - 0x800);
}

WRITE8_MEMBER(tceptor_state::tceptor_bg_scroll_w)
{
	switch (offset)
	{
	case 0:
		m_bg1_scroll_x &= 0xff;
		m_bg1_scroll_x |= data << 8;
		break;
	case 1:
		m_bg1_scroll_x &= 0xff00;
		m_bg1_scroll_x |= data;
		break;
	case 2:
		m_bg1_scroll_y = data;
		break;

	case 4:
		m_bg2_scroll_x &= 0xff;
		m_bg2_scroll_x |= data << 8;
		break;
	case 5:
		m_bg2_scroll_x &= 0xff00;
		m_bg2_scroll_x |= data;
		break;
	case 6:
		m_bg2_scroll_y = data;
		break;
	}
}


/*******************************************************************/

void tceptor_state::decode_bg(const char * region)
{
	static const gfx_layout bg_layout =
	{
		8, 8,
		2048,
		3,
		{ 0x40000+4, 0, 4 },
		{ 0, 1, 2, 3, 8, 9, 10, 11 },
		{ 0, 16, 32, 48, 64, 80, 96, 112 },
		128
	};

	int gfx_index = m_bg;
	UINT8 *src = memregion(region)->base() + 0x8000;
	int len = 0x8000;
	int i;

	dynamic_buffer buffer(len);

	/* expand rom tc2-19.10d */
	for (i = 0; i < len / 2; i++)
	{
		buffer[i*2+1] = src[i] & 0x0f;
		buffer[i*2] = (src[i] & 0xf0) >> 4;
	}

	memcpy(src, &buffer[0], len);

	/* decode the graphics */
	m_gfxdecode->set_gfx(gfx_index, global_alloc(gfx_element(m_palette, bg_layout, memregion(region)->base(), 0, 64, 0x0a00)));
}

void tceptor_state::decode_sprite(int gfx_index, const gfx_layout *layout, const void *data)
{
	/* decode the graphics */
	m_gfxdecode->set_gfx(gfx_index, global_alloc(gfx_element(m_palette, *layout, (const UINT8 *)data, 0, 64, 1024)));
}

// fix sprite order
void tceptor_state::decode_sprite16(const char * region)
{
	static const gfx_layout spr16_layout =
	{
		16, 16,
		512,
		4,
		{ 0x00000, 0x00004, 0x40000, 0x40004 },
		{
			0*8, 0*8+1, 0*8+2, 0*8+3, 1*8, 1*8+1, 1*8+2, 1*8+3,
			2*8, 2*8+1, 2*8+2, 2*8+3, 3*8, 3*8+1, 3*8+2, 3*8+3
		},
		{
				0*2*16,  1*2*16,  2*2*16,  3*2*16,  4*2*16,  5*2*16,  6*2*16,  7*2*16,
				8*2*16,  9*2*16, 10*2*16, 11*2*16, 12*2*16, 13*2*16, 14*2*16, 15*2*16
		},
		2*16*16
	};

	UINT8 *src = memregion(region)->base();
	int len = memregion(region)->bytes();
	UINT8 *dst;
	int i, y;

	dst = auto_alloc_array(machine(), UINT8, len);

	for (i = 0; i < len / (4*4*16); i++)
		for (y = 0; y < 16; y++)
		{
			memcpy(&dst[(i*4 + 0) * (2*16*16/8) + y * (2*16/8)],
					&src[i * (2*32*32/8) + y * (2*32/8)],
					4);
			memcpy(&dst[(i*4 + 1) * (2*16*16/8) + y * (2*16/8)],
					&src[i * (2*32*32/8) + y * (2*32/8) + (4*8/8)],
					4);
			memcpy(&dst[(i*4 + 2) * (2*16*16/8) + y * (2*16/8)],
					&src[i * (2*32*32/8) + y * (2*32/8) + (16*2*32/8)],
					4);
			memcpy(&dst[(i*4 + 3) * (2*16*16/8) + y * (2*16/8)],
					&src[i * (2*32*32/8) + y * (2*32/8) + (4*8/8) + (16*2*32/8)],
					4);
		}

	decode_sprite(m_sprite16, &spr16_layout, dst);
}

// fix sprite order
void tceptor_state::decode_sprite32(const char * region)
{
	static const gfx_layout spr32_layout =
	{
		32, 32,
		1024,
		4,
		{ 0x000000, 0x000004, 0x200000, 0x200004 },
		{
			0*8, 0*8+1, 0*8+2, 0*8+3, 1*8, 1*8+1, 1*8+2, 1*8+3,
			2*8, 2*8+1, 2*8+2, 2*8+3, 3*8, 3*8+1, 3*8+2, 3*8+3,
			4*8, 4*8+1, 4*8+2, 4*8+3, 5*8, 5*8+1, 5*8+2, 5*8+3,
			6*8, 6*8+1, 6*8+2, 6*8+3, 7*8, 7*8+1, 7*8+2, 7*8+3
		},
		{
				0*2*32,  1*2*32,  2*2*32,  3*2*32,  4*2*32,  5*2*32,  6*2*32,  7*2*32,
				8*2*32,  9*2*32, 10*2*32, 11*2*32, 12*2*32, 13*2*32, 14*2*32, 15*2*32,
			16*2*32, 17*2*32, 18*2*32, 19*2*32, 20*2*32, 21*2*32, 22*2*32, 23*2*32,
			24*2*32, 25*2*32, 26*2*32, 27*2*32, 28*2*32, 29*2*32, 30*2*32, 31*2*32
		},
		2*32*32
	};

	UINT8 *src = memregion(region)->base();
	int len = memregion(region)->bytes();
	int total = spr32_layout.total;
	int size = spr32_layout.charincrement / 8;
	UINT8 *dst;
	int i;

	dst = auto_alloc_array(machine(), UINT8, len);

	memset(dst, 0, len);

	for (i = 0; i < total; i++)
	{
		int code;

		code = (i & 0x07f) | ((i & 0x180) << 1) | 0x80;
		code &= ~((i & 0x200) >> 2);

		memcpy(&dst[size * (i + 0)],     &src[size * (code + 0)],     size);
		memcpy(&dst[size * (i + total)], &src[size * (code + total)], size);
	}

	decode_sprite(m_sprite32, &spr32_layout, dst);
}

void tceptor_state::video_start()
{
	int gfx_index;

	m_sprite_ram_buffered = auto_alloc_array_clear(machine(), UINT16, 0x200/2);

	/* find first empty slot to decode gfx */
	for (gfx_index = 0; gfx_index < MAX_GFX_ELEMENTS; gfx_index++)
		if (m_gfxdecode->gfx(gfx_index) == nullptr)
			break;
	assert(gfx_index + 4 <= MAX_GFX_ELEMENTS);

	m_bg = gfx_index++;
	decode_bg("gfx2");

	m_sprite16 = gfx_index++;
	decode_sprite16("gfx3");

	m_sprite32 = gfx_index++;
	decode_sprite32("gfx4");

	/* allocate temp bitmaps */
	m_2dscreen->register_screen_bitmap(m_temp_bitmap);

	m_c45_road->set_transparent_color(m_palette->pen_indirect(0xfff));

	m_tx_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(tceptor_state::get_tx_tile_info),this), TILEMAP_SCAN_COLS,  8, 8, 34, 28);

	m_tx_tilemap->set_scrollx(0, -2*8);
	m_tx_tilemap->set_scrolly(0, 0);
	m_tx_tilemap->configure_groups(*m_gfxdecode->gfx(0), 7);

	m_bg1_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(tceptor_state::get_bg1_tile_info),this), TILEMAP_SCAN_ROWS,  8, 8, 64, 32);
	m_bg2_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(tceptor_state::get_bg2_tile_info),this), TILEMAP_SCAN_ROWS,  8, 8, 64, 32);

	save_pointer(NAME(m_sprite_ram_buffered), 0x200 / 2);
	save_item(NAME(m_bg1_scroll_x));
	save_item(NAME(m_bg1_scroll_y));
	save_item(NAME(m_bg2_scroll_x));
	save_item(NAME(m_bg2_scroll_y));
}


/*******************************************************************/

/*
    Sprite data format

    000: zzzzzzBB BTTTTTTT
    002: ZZZZZZPP PPCCCCCC
    100: fFL---YY YYYYYYYY
    102: ------XX XXXXXXXX

    B: bank
    T: number
    P: priority
    C: color
    X: x
    Y: y
    L: large sprite
    F: flip x
    f: flip y
    Z: zoom x
    z: zoom y
*/

void tceptor_state::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, int sprite_priority)
{
	UINT16 *mem1 = &m_sprite_ram_buffered[0x000/2];
	UINT16 *mem2 = &m_sprite_ram_buffered[0x100/2];
	int need_mask = 0;
	int i;

	for (i = 0; i < 0x100; i += 2)
	{
		int scalex = (mem1[1 + i] & 0xfc00) << 1;
		int scaley = (mem1[0 + i] & 0xfc00) << 1;
		int pri = 7 - ((mem1[1 + i] & 0x3c0) >> 6);

		if (pri == sprite_priority && scalex && scaley)
		{
			int x = mem2[1 + i] & 0x3ff;
			int y = 512 - (mem2[0 + i] & 0x3ff);
			int flipx = mem2[0 + i] & 0x4000;
			int flipy = mem2[0 + i] & 0x8000;
			int color = mem1[1 + i] & 0x3f;
			int gfx;
			int code;

			if (mem2[0 + i] & 0x2000)
			{
				gfx = m_sprite32;
				code = mem1[0 + i] & 0x3ff;

			}
			else
			{
				gfx = m_sprite16;
				code = mem1[0 + i] & 0x1ff;
				scaley *= 2;
			}

			if (m_is_mask_spr[color])
			{
				if (!need_mask)
					// backup previous bitmap
					copybitmap(m_temp_bitmap, bitmap, 0, 0, 0, 0, cliprect);

				need_mask = 1;
			}

			// round off
			scalex += 0x800;
			scaley += 0x800;

			x -= 64;
			y -= 78;


						m_gfxdecode->gfx(gfx)->zoom_transmask(bitmap,
						cliprect,
						code,
						color,
						flipx, flipy,
						x, y,
						scalex,
						scaley,
						m_palette->transpen_mask(*m_gfxdecode->gfx(gfx), color, SPR_TRANS_COLOR));
		}
	}

	/* if SPR_MASK_COLOR pen is used, restore pixels from previous bitmap */
	if (need_mask)
	{
		int x, y;

		for (x = cliprect.min_x; x <= cliprect.max_x; x++)
			for (y = cliprect.min_y; y <= cliprect.max_y; y++)
				if (m_palette->pen_indirect(bitmap.pix16(y, x)) == SPR_MASK_COLOR)
					// restore pixel
					bitmap.pix16(y, x) = m_temp_bitmap.pix16(y, x);
	}
}


UINT32 tceptor_state::screen_update_tceptor_2d(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	rectangle rect;
	int pri;
	int bg_center = 144 - ((((m_bg1_scroll_x + m_bg2_scroll_x ) & 0x1ff) - 288) / 2);

	// left background
	rect = cliprect;
	rect.max_x = bg_center;
	m_bg1_tilemap->set_scrollx(0, m_bg1_scroll_x + 12);
	m_bg1_tilemap->set_scrolly(0, m_bg1_scroll_y + 20); //32?
	m_bg1_tilemap->draw(screen, bitmap, rect, 0, 0);

	// right background
	rect.min_x = bg_center;
	rect.max_x = cliprect.max_x;
	m_bg2_tilemap->set_scrollx(0, m_bg2_scroll_x + 20);
	m_bg2_tilemap->set_scrolly(0, m_bg2_scroll_y + 20); // 32?
	m_bg2_tilemap->draw(screen, bitmap, rect, 0, 0);

	for (pri = 0; pri < 8; pri++)
	{
		m_c45_road->draw(bitmap, cliprect, pri * 2);
		m_c45_road->draw(bitmap, cliprect, pri * 2 + 1);
		draw_sprites(bitmap, cliprect, pri);
	}

	m_tx_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}

UINT32 tceptor_state::screen_update_tceptor_3d_left(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	if ((screen.frame_number() & 1) == 1)
		return UPDATE_HAS_NOT_CHANGED;
	return screen_update_tceptor_2d(screen, bitmap, cliprect);
}

UINT32 tceptor_state::screen_update_tceptor_3d_right(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	if ((screen.frame_number() & 1) == 0)
		return UPDATE_HAS_NOT_CHANGED;
	return screen_update_tceptor_2d(screen, bitmap, cliprect);
}


void tceptor_state::screen_eof_tceptor(screen_device &screen, bool state)
{
	// rising edge
	if (state)
	{
		memcpy(m_sprite_ram_buffered, m_sprite_ram, 0x200);
	}
}
