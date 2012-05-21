/***************************************************************************

Atari Starship 1 video emulation

***************************************************************************/

#include "emu.h"
#include "includes/starshp1.h"


static void set_pens(starshp1_state *state, colortable_t *colortable)
{
	colortable_palette_set_color(colortable, state->m_inverse ? 7 : 0, MAKE_RGB(0x00, 0x00, 0x00));
	colortable_palette_set_color(colortable, state->m_inverse ? 6 : 1, MAKE_RGB(0x1e, 0x1e, 0x1e));
	colortable_palette_set_color(colortable, state->m_inverse ? 5 : 2, MAKE_RGB(0x4e, 0x4e, 0x4e));
	colortable_palette_set_color(colortable, state->m_inverse ? 4 : 3, MAKE_RGB(0x6c, 0x6c, 0x6c));
	colortable_palette_set_color(colortable, state->m_inverse ? 3 : 4, MAKE_RGB(0x93, 0x93, 0x93));
	colortable_palette_set_color(colortable, state->m_inverse ? 2 : 5, MAKE_RGB(0xb1, 0xb1, 0xb1));
	colortable_palette_set_color(colortable, state->m_inverse ? 1 : 6, MAKE_RGB(0xe1, 0xe1, 0xe1));
	colortable_palette_set_color(colortable, state->m_inverse ? 0 : 7, MAKE_RGB(0xff, 0xff, 0xff));
}


PALETTE_INIT( starshp1 )
{
	int i;

	static const UINT16 colortable_source[] =
	{
		0, 3,       /* 0x00 - 0x01 - alpha numerics */
		0, 2,       /* 0x02 - 0x03 - sprites (Z=0) */
		0, 5,       /* 0x04 - 0x05 - sprites (Z=1) */
		0, 2, 4, 6, /* 0x06 - 0x09 - spaceship (EXPLODE=0) */
		0, 6, 6, 7, /* 0x0a - 0x0d - spaceship (EXPLODE=1) */
		5, 2,		/* 0x0e - 0x0f - star field */
		7,			/* 0x10        - phasor */
		5, 7		/* 0x11        - circle */
	};

	/* allocate the colortable */
	machine.colortable = colortable_alloc(machine, 8);

	for (i = 0; i < sizeof(colortable_source) / sizeof(colortable_source[0]); i++)
		colortable_entry_set_value(machine.colortable, i, colortable_source[i]);
}


static TILE_GET_INFO( get_tile_info )
{
	starshp1_state *state = machine.driver_data<starshp1_state>();
	UINT8 code = state->m_playfield_ram[tile_index];

	SET_TILE_INFO(0, code & 0x3f, 0, 0);
}


VIDEO_START( starshp1 )
{
	starshp1_state *state = machine.driver_data<starshp1_state>();
	UINT16 val = 0;

	int i;

	state->m_bg_tilemap = tilemap_create(machine, get_tile_info, tilemap_scan_rows,  16, 8, 32, 32);

	state->m_bg_tilemap->set_transparent_pen(0);

	state->m_bg_tilemap->set_scrollx(0, -8);

	state->m_LSFR = auto_alloc_array(machine, UINT16, 0x10000);

	for (i = 0; i < 0x10000; i++)
	{
		int bit = (val >> 0xf) ^
				  (val >> 0xc) ^
				  (val >> 0x7) ^
				  (val >> 0x1) ^ 1;

		state->m_LSFR[i] = val;

		val = (val << 1) | (bit & 1);
	}

	machine.primary_screen->register_screen_bitmap(state->m_helper);
}


READ8_MEMBER(starshp1_state::starshp1_rng_r)
{
	int width = machine().primary_screen->width();
	int height = machine().primary_screen->height();
	int x = machine().primary_screen->hpos();
	int y = machine().primary_screen->vpos();

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
     * move the zoomed &spaceship sprite over the top and left
     * edges of the screen. These additional values are used
     * to compensate for this. Technically, they cut off the
     * first columns and rows of the &spaceship sprite, but in
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


static void draw_starfield(starshp1_state *state, bitmap_ind16 &bitmap)
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
		const UINT16* p = state->m_LSFR + (UINT16) (512 * y);

		UINT16* pLine = &bitmap.pix16(y);

		for (x = 0; x < bitmap.width(); x++)
			if ((p[x] & 0x5b56) == 0x5b44)
				pLine[x] = (p[x] & 0x0400) ? 0x0e : 0x0f;
	}
}


static int get_sprite_hpos(starshp1_state *state, int i)
{
	return 2 * (state->m_hpos_ram[i] ^ 0xff);
}
static int get_sprite_vpos(starshp1_state *state, int i)
{
	return 1 * (state->m_vpos_ram[i] - 0x07);
}


static void draw_sprites(running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	starshp1_state *state = machine.driver_data<starshp1_state>();
	int i;

	for (i = 0; i < 14; i++)
	{
		int code = (state->m_obj_ram[i] & 0xf) ^ 0xf;

		drawgfx_transpen(bitmap, cliprect, machine.gfx[1],
			code % 8,
			code / 8,
			0, 0,
			get_sprite_hpos(state, i),
			get_sprite_vpos(state, i), 0);
	}
}


static void draw_spaceship(running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	starshp1_state *state = machine.driver_data<starshp1_state>();
	double scaler = -5 * log(1 - state->m_ship_size / 256.0); /* ? */

	unsigned xzoom = 2 * 0x10000 * scaler;
	unsigned yzoom = 1 * 0x10000 * scaler;

	int x = get_sprite_hpos(state, 14);
	int y = get_sprite_vpos(state, 14);

	if (x <= 0)
		x -= (xzoom * state->m_ship_hoffset) >> 16;

	if (y <= 0)
		y -= (yzoom * state->m_ship_voffset) >> 16;

	drawgfxzoom_transpen(bitmap, cliprect, machine.gfx[2],
		state->m_ship_picture & 0x03,
		state->m_ship_explode,
		state->m_ship_picture & 0x80, 0,
		x, y,
		xzoom, yzoom, 0);
}


static void draw_phasor(starshp1_state *state, bitmap_ind16 &bitmap)
{
	int i;

	for (i = 128; i < 240; i++)
		if (i >= get_sprite_vpos(state, 13))
		{
			bitmap.pix16(i, 2 * i + 0) = 0x10;
			bitmap.pix16(i, 2 * i + 1) = 0x10;
			bitmap.pix16(i, 2 * (255 - i) + 0) = 0x10;
			bitmap.pix16(i, 2 * (255 - i) + 1) = 0x10;
		}
}


static int get_radius(starshp1_state *state)
{
	return 6 * sqrt((double)state->m_circle_size);  /* size calibrated by hand */
}
static int get_circle_hpos(starshp1_state *state)
{
	return 2 * (3 * state->m_circle_hpos / 2 - 64);
}
static int get_circle_vpos(starshp1_state *state)
{
	return 1 * (3 * state->m_circle_vpos / 2 - 64);
}


static void draw_circle_line(running_machine &machine, bitmap_ind16 &bitmap, int x, int y, int l)
{
	starshp1_state *state = machine.driver_data<starshp1_state>();
	if (y >= 0 && y <= bitmap.height() - 1)
	{
		const UINT16* p = state->m_LSFR + (UINT16) (512 * y);

		UINT16* pLine = &bitmap.pix16(y);

		int h1 = x - 2 * l;
		int h2 = x + 2 * l;

		if (h1 < 0)
			h1 = 0;
		if (h2 > bitmap.width() - 1)
			h2 = bitmap.width() - 1;

		for (x = h1; x <= h2; x++)
			if (state->m_circle_mod)
			{
				if (p[x] & 1)
					pLine[x] = 0x11;
			}
			else
				pLine[x] = 0x12;
	}
}


static void draw_circle(running_machine &machine, bitmap_ind16 &bitmap)
{
	starshp1_state *state = machine.driver_data<starshp1_state>();
	int cx = get_circle_hpos(state);
	int cy = get_circle_vpos(state);

	int x = 0;
	int y = get_radius(state);

	/* Bresenham's circle algorithm */

	int d = 3 - 2 * get_radius(state);

	while (x <= y)
	{
		draw_circle_line(machine, bitmap, cx, cy - x, y);
		draw_circle_line(machine, bitmap, cx, cy + x, y);
		draw_circle_line(machine, bitmap, cx, cy - y, x);
		draw_circle_line(machine, bitmap, cx, cy + y, x);

		x++;

		if (d < 0)
			d += 4 * x + 6;
		else
			d += 4 * (x - y--) + 10;
	}
}


static int spaceship_collision(running_machine &machine, bitmap_ind16 &bitmap, const rectangle &rect)
{
	starshp1_state *state = machine.driver_data<starshp1_state>();
	int x;
	int y;

	for (y = rect.min_y; y <= rect.max_y; y++)
	{
		const UINT16* pLine = &state->m_helper.pix16(y);

		for (x = rect.min_x; x <= rect.max_x; x++)
			if (pLine[x] != 0)
				return 1;
	}

	return 0;
}


static int point_in_circle(int x, int y, int center_x, int center_y, int r)
{
	int dx = abs(x - center_x) / 2;
	int dy = abs(y - center_y) / 1;

	return dx * dx + dy * dy < r * r;
}


static int circle_collision(starshp1_state *state, const rectangle &rect)
{
	int center_x = get_circle_hpos(state);
	int center_y = get_circle_vpos(state);

	int r = get_radius(state);

	return point_in_circle(rect.min_x, rect.min_y, center_x, center_y, r) ||
		   point_in_circle(rect.min_x, rect.max_y, center_x, center_y, r) ||
		   point_in_circle(rect.max_x, rect.min_y, center_x, center_y, r) ||
		   point_in_circle(rect.max_x, rect.max_y, center_x, center_y, r);
}


SCREEN_UPDATE_IND16( starshp1 )
{
	starshp1_state *state = screen.machine().driver_data<starshp1_state>();
	set_pens(state, screen.machine().colortable);

	bitmap.fill(0, cliprect);

	if (state->m_starfield_kill == 0)
		draw_starfield(state, bitmap);

	draw_sprites(screen.machine(), bitmap, cliprect);

	if (state->m_circle_kill == 0 && state->m_circle_mod != 0)
		draw_circle(screen.machine(), bitmap);

	if (state->m_attract == 0)
		draw_spaceship(screen.machine(), bitmap, cliprect);

	if (state->m_circle_kill == 0 && state->m_circle_mod == 0)
		draw_circle(screen.machine(), bitmap);

	state->m_bg_tilemap->draw(bitmap, cliprect, 0, 0);

	if (state->m_phasor != 0)
		draw_phasor(state, bitmap);

	return 0;
}


SCREEN_VBLANK( starshp1 )
{
	// rising edge
	if (vblank_on)
	{
		starshp1_state *state = screen.machine().driver_data<starshp1_state>();
		rectangle rect;
		const rectangle &visarea = screen.machine().primary_screen->visible_area();

		rect.min_x = get_sprite_hpos(state, 13);
		rect.min_y = get_sprite_vpos(state, 13);
		rect.max_x = rect.min_x + screen.machine().gfx[1]->width - 1;
		rect.max_y = rect.min_y + screen.machine().gfx[1]->height - 1;

		rect &= state->m_helper.cliprect();

		state->m_helper.fill(0, visarea);

		if (state->m_attract == 0)
			draw_spaceship(screen.machine(), state->m_helper, visarea);

		if (circle_collision(state, visarea))
			state->m_collision_latch |= 1;

		if (circle_collision(state, rect))
			state->m_collision_latch |= 2;

		if (spaceship_collision(screen.machine(), state->m_helper, rect))
			state->m_collision_latch |= 4;

		if (spaceship_collision(screen.machine(), state->m_helper, visarea))
			state->m_collision_latch |= 8;
	}
}
