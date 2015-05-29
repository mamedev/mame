// license:???
// copyright-holders:Frank Palazzolo, Stefan Jokisch
/***************************************************************************

Atari Starship 1 video emulation

***************************************************************************/

#include "emu.h"
#include "includes/starshp1.h"


void starshp1_state::set_pens()
{
	m_palette->set_indirect_color(m_inverse ? 7 : 0, rgb_t(0x00, 0x00, 0x00));
	m_palette->set_indirect_color(m_inverse ? 6 : 1, rgb_t(0x1e, 0x1e, 0x1e));
	m_palette->set_indirect_color(m_inverse ? 5 : 2, rgb_t(0x4e, 0x4e, 0x4e));
	m_palette->set_indirect_color(m_inverse ? 4 : 3, rgb_t(0x6c, 0x6c, 0x6c));
	m_palette->set_indirect_color(m_inverse ? 3 : 4, rgb_t(0x93, 0x93, 0x93));
	m_palette->set_indirect_color(m_inverse ? 2 : 5, rgb_t(0xb1, 0xb1, 0xb1));
	m_palette->set_indirect_color(m_inverse ? 1 : 6, rgb_t(0xe1, 0xe1, 0xe1));
	m_palette->set_indirect_color(m_inverse ? 0 : 7, rgb_t(0xff, 0xff, 0xff));
}


PALETTE_INIT_MEMBER(starshp1_state, starshp1)
{
	int i;

	static const UINT16 colortable_source[] =
	{
		0, 3,       /* 0x00 - 0x01 - alpha numerics */
		0, 2,       /* 0x02 - 0x03 - sprites (Z=0) */
		0, 5,       /* 0x04 - 0x05 - sprites (Z=1) */
		0, 2, 4, 6, /* 0x06 - 0x09 - spaceship (EXPLODE=0) */
		0, 6, 6, 7, /* 0x0a - 0x0d - spaceship (EXPLODE=1) */
		5, 2,       /* 0x0e - 0x0f - star field */
		7,          /* 0x10        - phasor */
		5, 7        /* 0x11        - circle */
	};

	for (i = 0; i < ARRAY_LENGTH(colortable_source); i++)
		palette.set_pen_indirect(i, colortable_source[i]);
}


TILE_GET_INFO_MEMBER(starshp1_state::get_tile_info)
{
	UINT8 code = m_playfield_ram[tile_index];

	SET_TILE_INFO_MEMBER(0, code & 0x3f, 0, 0);
}


void starshp1_state::video_start()
{
	UINT16 val = 0;

	int i;

	m_bg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(starshp1_state::get_tile_info),this), TILEMAP_SCAN_ROWS,  16, 8, 32, 32);

	m_bg_tilemap->set_transparent_pen(0);

	m_bg_tilemap->set_scrollx(0, -8);

	m_LSFR = auto_alloc_array(machine(), UINT16, 0x10000);

	for (i = 0; i < 0x10000; i++)
	{
		int bit = (val >> 0xf) ^
					(val >> 0xc) ^
					(val >> 0x7) ^
					(val >> 0x1) ^ 1;

		m_LSFR[i] = val;

		val = (val << 1) | (bit & 1);
	}

	m_screen->register_screen_bitmap(m_helper);
}


READ8_MEMBER(starshp1_state::starshp1_rng_r)
{
	int width = m_screen->width();
	int height = m_screen->height();
	int x = m_screen->hpos();
	int y = m_screen->vpos();

	/* the LFSR is only running in the non-blank region
	   of the screen, so this is not quite right */
	if (x > width - 1)
		x = width - 1;
	if (y > height - 1)
		y = height - 1;

	return m_LSFR[x + (UINT16) (512 * y)];
}


WRITE8_MEMBER(starshp1_state::starshp1_ssadd_w)
{
	/*
	 * The range of sprite position values doesn't suffice to
	 * move the zoomed spaceship sprite over the top and left
	 * edges of the screen. These additional values are used
	 * to compensate for this. Technically, they cut off the
	 * first columns and rows of the spaceship sprite, but in
	 * practice they work like offsets in zoomed pixels.
	 */

	m_ship_voffset = ((offset & 0xf0) >> 4);
	m_ship_hoffset = ((offset & 0x0f) << 2) | (data & 3);
}


WRITE8_MEMBER(starshp1_state::starshp1_sspic_w)
{
	/*
	 * Some mysterious game code at address $2CCE is causing
	 * erratic images in the target explosion sequence. The
	 * following condition is a hack to filter these images.
	 */

	if (data != 0x87)
		m_ship_picture = data;
}


WRITE8_MEMBER(starshp1_state::starshp1_playfield_w)
{
	if (m_mux != 0)
	{
		offset ^= 0x1f;
		m_playfield_ram[offset] = data;
		m_bg_tilemap->mark_tile_dirty(offset);
	}
}


void starshp1_state::draw_starfield(bitmap_ind16 &bitmap)
{
	/*
	 * The LSFR is reset once per frame at the position of
	 * sprite 15. This behavior is quite pointless and not
	 * really needed by the game. Not emulated.
	 */

	int x;
	int y;

	for (y = 0; y < bitmap.height(); y++)
	{
		const UINT16* p = m_LSFR + (UINT16) (512 * y);

		UINT16* pLine = &bitmap.pix16(y);

		for (x = 0; x < bitmap.width(); x++)
			if ((p[x] & 0x5b56) == 0x5b44)
				pLine[x] = (p[x] & 0x0400) ? 0x0e : 0x0f;
	}
}


int starshp1_state::get_sprite_hpos(int i)
{
	return 2 * (m_hpos_ram[i] ^ 0xff);
}
int starshp1_state::get_sprite_vpos(int i)
{
	return 1 * (m_vpos_ram[i] - 0x07);
}


void starshp1_state::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int i;

	for (i = 0; i < 14; i++)
	{
		int code = (m_obj_ram[i] & 0xf) ^ 0xf;

		m_gfxdecode->gfx(1)->transpen(bitmap,cliprect,
			code % 8,
			code / 8,
			0, 0,
			get_sprite_hpos(i),
			get_sprite_vpos(i), 0);
	}
}


void starshp1_state::draw_spaceship(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	double scaler = -5 * log(1 - m_ship_size / 256.0); /* ? */

	unsigned xzoom = 2 * 0x10000 * scaler;
	unsigned yzoom = 1 * 0x10000 * scaler;

	int x = get_sprite_hpos(14);
	int y = get_sprite_vpos(14);

	if (x <= 0)
		x -= (xzoom * m_ship_hoffset) >> 16;

	if (y <= 0)
		y -= (yzoom * m_ship_voffset) >> 16;

	m_gfxdecode->gfx(2)->zoom_transpen(bitmap,cliprect,
		m_ship_picture & 0x03,
		m_ship_explode,
		m_ship_picture & 0x80, 0,
		x, y,
		xzoom, yzoom, 0);
}


void starshp1_state::draw_phasor(bitmap_ind16 &bitmap)
{
	int i;

	for (i = 128; i < 240; i++)
		if (i >= get_sprite_vpos(13))
		{
			bitmap.pix16(i, 2 * i + 0) = 0x10;
			bitmap.pix16(i, 2 * i + 1) = 0x10;
			bitmap.pix16(i, 2 * (255 - i) + 0) = 0x10;
			bitmap.pix16(i, 2 * (255 - i) + 1) = 0x10;
		}
}


int starshp1_state::get_radius()
{
	return 6 * sqrt((double)m_circle_size);  /* size calibrated by hand */
}
int starshp1_state::get_circle_hpos()
{
	return 2 * (3 * m_circle_hpos / 2 - 64);
}
int starshp1_state::get_circle_vpos()
{
	return 1 * (3 * m_circle_vpos / 2 - 64);
}


void starshp1_state::draw_circle_line(bitmap_ind16 &bitmap, int x, int y, int l)
{
	if (y >= 0 && y <= bitmap.height() - 1)
	{
		const UINT16* p = m_LSFR + (UINT16) (512 * y);

		UINT16* pLine = &bitmap.pix16(y);

		int h1 = x - 2 * l;
		int h2 = x + 2 * l;

		if (h1 < 0)
			h1 = 0;
		if (h2 > bitmap.width() - 1)
			h2 = bitmap.width() - 1;

		for (x = h1; x <= h2; x++)
			if (m_circle_mod)
			{
				if (p[x] & 1)
					pLine[x] = 0x11;
			}
			else
				pLine[x] = 0x12;
	}
}


void starshp1_state::draw_circle(bitmap_ind16 &bitmap)
{
	int cx = get_circle_hpos();
	int cy = get_circle_vpos();

	int x = 0;
	int y = get_radius();

	/* Bresenham's circle algorithm */

	int d = 3 - 2 * get_radius();

	while (x <= y)
	{
		draw_circle_line(bitmap, cx, cy - x, y);
		draw_circle_line(bitmap, cx, cy + x, y);
		draw_circle_line(bitmap, cx, cy - y, x);
		draw_circle_line(bitmap, cx, cy + y, x);

		x++;

		if (d < 0)
			d += 4 * x + 6;
		else
			d += 4 * (x - y--) + 10;
	}
}


int starshp1_state::spaceship_collision(bitmap_ind16 &bitmap, const rectangle &rect)
{
	int x;
	int y;

	for (y = rect.min_y; y <= rect.max_y; y++)
	{
		const UINT16* pLine = &m_helper.pix16(y);

		for (x = rect.min_x; x <= rect.max_x; x++)
			if (pLine[x] != 0)
				return 1;
	}

	return 0;
}


int starshp1_state::point_in_circle(int x, int y, int center_x, int center_y, int r)
{
	int dx = abs(x - center_x) / 2;
	int dy = abs(y - center_y) / 1;

	return dx * dx + dy * dy < r * r;
}


int starshp1_state::circle_collision(const rectangle &rect)
{
	int center_x = get_circle_hpos();
	int center_y = get_circle_vpos();

	int r = get_radius();

	return point_in_circle(rect.min_x, rect.min_y, center_x, center_y, r) ||
			point_in_circle(rect.min_x, rect.max_y, center_x, center_y, r) ||
			point_in_circle(rect.max_x, rect.min_y, center_x, center_y, r) ||
			point_in_circle(rect.max_x, rect.max_y, center_x, center_y, r);
}


UINT32 starshp1_state::screen_update_starshp1(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	set_pens();

	bitmap.fill(0, cliprect);

	if (m_starfield_kill == 0)
		draw_starfield(bitmap);

	draw_sprites(bitmap, cliprect);

	if (m_circle_kill == 0 && m_circle_mod != 0)
		draw_circle(bitmap);

	if (m_attract == 0)
		draw_spaceship(bitmap, cliprect);

	if (m_circle_kill == 0 && m_circle_mod == 0)
		draw_circle(bitmap);

	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);

	if (m_phasor != 0)
		draw_phasor(bitmap);

	return 0;
}


void starshp1_state::screen_eof_starshp1(screen_device &screen, bool state)
{
	// rising edge
	if (state)
	{
		rectangle rect;
		const rectangle &visarea = m_screen->visible_area();

		rect.min_x = get_sprite_hpos(13);
		rect.min_y = get_sprite_vpos(13);
		rect.max_x = rect.min_x + m_gfxdecode->gfx(1)->width() - 1;
		rect.max_y = rect.min_y + m_gfxdecode->gfx(1)->height() - 1;

		rect &= m_helper.cliprect();

		m_helper.fill(0, visarea);

		if (m_attract == 0)
			draw_spaceship(m_helper, visarea);

		if (circle_collision(visarea))
			m_collision_latch |= 1;

		if (circle_collision(rect))
			m_collision_latch |= 2;

		if (spaceship_collision(m_helper, rect))
			m_collision_latch |= 4;

		if (spaceship_collision(m_helper, visarea))
			m_collision_latch |= 8;
	}
}
