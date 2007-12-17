/***************************************************************************

Atari Starship 1 video emulation

***************************************************************************/

#include "driver.h"
#include "includes/starshp1.h"
#include <math.h>

UINT8* starshp1_playfield_ram;
UINT8* starshp1_hpos_ram;
UINT8* starshp1_vpos_ram;
UINT8* starshp1_obj_ram;

int starshp1_ship_explode;
int starshp1_ship_picture;
int starshp1_ship_hoffset;
int starshp1_ship_voffset;
int starshp1_ship_size;

int starshp1_circle_kill;
int starshp1_circle_mod;
int starshp1_circle_hpos;
int starshp1_circle_vpos;
int starshp1_circle_size;

int starshp1_starfield_kill;
int starshp1_phasor;
int starshp1_collision_latch;
int starshp1_mux;

static UINT16* LSFR;

static mame_bitmap* helper;

static tilemap* bg_tilemap;


static TILE_GET_INFO( get_tile_info )
{
	UINT8 code = starshp1_playfield_ram[tile_index];

	SET_TILE_INFO(0, code & 0x3f, 0, 0);
}


VIDEO_START( starshp1 )
{
	UINT16 val = 0;

	int i;

	bg_tilemap = tilemap_create(get_tile_info, tilemap_scan_rows, TILEMAP_TYPE_PEN, 16, 8, 32, 32);

	tilemap_set_transparent_pen(bg_tilemap, 0);

	tilemap_set_scrollx(bg_tilemap, 0, -8);

	LSFR = auto_malloc(0x20000);

	for (i = 0; i < 0x10000; i++)
	{
		int bit =
			(val >> 0xF) ^
			(val >> 0xC) ^
			(val >> 0x7) ^
			(val >> 0x1) ^ 1;

		LSFR[i] = val;

		val = (val << 1) | (bit & 1);
	}

	helper = auto_bitmap_alloc(machine->screen[0].width, machine->screen[0].height, machine->screen[0].format);
}


READ8_HANDLER( starshp1_rng_r )
{
	int x = video_screen_get_hpos(0);
	int y = video_screen_get_vpos(0);

	/* the LFSR is only running in the non-blank region
       of the screen, so this is not quite right */
	if (x > Machine->screen[0].width - 1)
		x = Machine->screen[0].width - 1;
	if (y > Machine->screen[0].height - 1)
		y = Machine->screen[0].height - 1;

	return LSFR[x + (UINT16) (512 * y)];
}


WRITE8_HANDLER( starshp1_ssadd_w )
{
	/*
     * The range of sprite position values doesn't suffice to
     * move the zoomed spaceship sprite over the top and left
     * edges of the screen. These additional values are used
     * to compensate for this. Technically, they cut off the
     * first columns and rows of the spaceship sprite, but in
     * practice they work like offsets in zoomed pixels.
     */

	starshp1_ship_voffset = ((offset & 0xf0) >> 4);
	starshp1_ship_hoffset = ((offset & 0x0f) << 2) | (data & 3);
}


WRITE8_HANDLER( starshp1_sspic_w )
{
	/*
     * Some mysterious game code at address $2CCE is causing
     * erratic images in the target explosion sequence. The
     * following condition is a hack to filter these images.
     */

	if (data != 0x87)
	{
		starshp1_ship_picture = data;
	}
}


WRITE8_HANDLER( starshp1_playfield_w )
{
	if (starshp1_mux != 0)
	{
		offset ^= 0x1f;
		starshp1_playfield_ram[offset] = data;
		tilemap_mark_tile_dirty(bg_tilemap, offset);
	}
}


static void draw_starfield(mame_bitmap* bitmap)
{
	/*
     * The LSFR is reset once per frame at the position of
     * sprite 15. This behavior is quite pointless and not
     * really needed by the game. Not emulated.
     */

	int x;
	int y;

	for (y = 0; y < bitmap->height; y++)
	{
		const UINT16* p = LSFR + (UINT16) (512 * y);

		UINT16* pLine = BITMAP_ADDR16(bitmap, y, 0);

		for (x = 0; x < bitmap->width; x++)
		{
			if ((p[x] & 0x5b56) == 0x5b44)
			{
				pLine[x] = (p[x] & 0x0400) ? 5 : 2;
			}
		}
	}
}


static int get_sprite_hpos(int i)
{
	return 2 * (starshp1_hpos_ram[i] ^ 0xff);
}
static int get_sprite_vpos(int i)
{
	return 1 * (starshp1_vpos_ram[i] - 0x07);
}


static void draw_sprites(running_machine *machine, mame_bitmap* bitmap, const rectangle* cliprect)
{
	int i;

	for (i = 0; i < 14; i++)
	{
		int code = (starshp1_obj_ram[i] & 0xf) ^ 0xf;

		drawgfx(bitmap, machine->gfx[1],
			code % 8,
			code / 8,
			0, 0,
			get_sprite_hpos(i),
			get_sprite_vpos(i),
			cliprect, TRANSPARENCY_PEN, 0);
	}
}


static void draw_spaceship(running_machine *machine, mame_bitmap* bitmap, const rectangle* cliprect)
{
	double scaler = -5 * log(1 - starshp1_ship_size / 256.0); /* ? */

	unsigned xzoom = 2 * 0x10000 * scaler;
	unsigned yzoom = 1 * 0x10000 * scaler;

	int x = get_sprite_hpos(14);
	int y = get_sprite_vpos(14);

	if (x <= 0)
	{
		x -= (xzoom * starshp1_ship_hoffset) >> 16;
	}
	if (y <= 0)
	{
		y -= (yzoom * starshp1_ship_voffset) >> 16;
	}

	drawgfxzoom(bitmap, machine->gfx[2],
		starshp1_ship_picture & 0x03,
		starshp1_ship_explode,
		starshp1_ship_picture & 0x80, 0,
		x, y,
		cliprect, TRANSPARENCY_PEN, 0,
		xzoom, yzoom);
}


static void draw_phasor(mame_bitmap* bitmap)
{
	int i;

	for (i = 128; i < 240; i++)
	{
		if (i >= get_sprite_vpos(13))
		{
			*BITMAP_ADDR16(bitmap, i, 2 * i + 0) = 7;
			*BITMAP_ADDR16(bitmap, i, 2 * i + 1) = 7;
			*BITMAP_ADDR16(bitmap, i, 2 * (255 - i) + 0) = 7;
			*BITMAP_ADDR16(bitmap, i, 2 * (255 - i) + 1) = 7;
		}
	}
}


static int get_radius(void)
{
	return 6 * sqrt(starshp1_circle_size);  /* size calibrated by hand */
}
static int get_circle_hpos(void)
{
	return 2 * (3 * starshp1_circle_hpos / 2 - 64);
}
static int get_circle_vpos(void)
{
	return 1 * (3 * starshp1_circle_vpos / 2 - 64);
}


static void draw_circle_line(mame_bitmap *bitmap, int x, int y, int l)
{
	if (y >= 0 && y <= bitmap->height - 1)
	{
		const UINT16* p = LSFR + (UINT16) (512 * y);

		UINT16* pLine = BITMAP_ADDR16(bitmap, y, 0);

		int h1 = x - 2 * l;
		int h2 = x + 2 * l;

		if (h1 < 0)
			h1 = 0;
		if (h2 > bitmap->width - 1)
			h2 = bitmap->width - 1;

		for (x = h1; x <= h2; x++)
		{
			if (starshp1_circle_mod)
			{
				if (p[x] & 1)
				{
					pLine[x] = 5;
				}
			}
			else
			{
				pLine[x] = 7;
			}
		}
	}
}


static void draw_circle(mame_bitmap* bitmap)
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
		{
			d += 4 * x + 6;
		}
		else
		{
			d += 4 * (x - y--) + 10;
		}
	}
}


static int spaceship_collision(mame_bitmap* bitmap, rectangle* rect)
{
	int x;
	int y;

	for (y = rect->min_y; y <= rect->max_y; y++)
	{
		const UINT16* pLine = BITMAP_ADDR16(helper, y, 0);

		for (x = rect->min_x; x <= rect->max_x; x++)
		{
			if (pLine[x] != 0)
			{
				return 1;
			}
		}
	}

	return 0;
}


static int point_in_circle(int x, int y, int center_x, int center_y, int r)
{
	int dx = abs(x - center_x) / 2;
	int dy = abs(y - center_y) / 1;

	return dx * dx + dy * dy < r * r;
}


static int circle_collision(rectangle* rect)
{
	int center_x = get_circle_hpos();
	int center_y = get_circle_vpos();

	int r = get_radius();

	return
		point_in_circle(rect->min_x, rect->min_y, center_x, center_y, r) ||
		point_in_circle(rect->min_x, rect->max_y, center_x, center_y, r) ||
		point_in_circle(rect->max_x, rect->min_y, center_x, center_y, r) ||
		point_in_circle(rect->max_x, rect->max_y, center_x, center_y, r);
}


VIDEO_UPDATE( starshp1 )
{
	fillbitmap(bitmap, machine->pens[0], cliprect);

	if (starshp1_starfield_kill == 0)
		draw_starfield(bitmap);

	draw_sprites(machine, bitmap, cliprect);

	if (starshp1_circle_kill == 0 && starshp1_circle_mod != 0)
		draw_circle(bitmap);

	if (starshp1_attract == 0)
		draw_spaceship(machine, bitmap, cliprect);

	if (starshp1_circle_kill == 0 && starshp1_circle_mod == 0)
		draw_circle(bitmap);

	tilemap_draw(bitmap, cliprect, bg_tilemap, 0, 0);

	if (starshp1_phasor != 0)
		draw_phasor(bitmap);
	return 0;
}


VIDEO_EOF( starshp1 )
{
	rectangle rect;

	rect.min_x = get_sprite_hpos(13);
	rect.min_y = get_sprite_vpos(13);
	rect.max_x = rect.min_x + machine->gfx[1]->width - 1;
	rect.max_y = rect.min_y + machine->gfx[1]->height - 1;

	if (rect.min_x < 0)
		rect.min_x = 0;
	if (rect.min_y < 0)
		rect.min_y = 0;
	if (rect.max_x > helper->width - 1)
		rect.max_x = helper->width - 1;
	if (rect.max_y > helper->height - 1)
		rect.max_y = helper->height - 1;

	fillbitmap(helper, machine->pens[0], &machine->screen[0].visarea);

	if (starshp1_attract == 0)
		draw_spaceship(machine, helper, &machine->screen[0].visarea);

	if (circle_collision(&machine->screen[0].visarea))
	{
		starshp1_collision_latch |= 1;
	}
	if (circle_collision(&rect))
	{
		starshp1_collision_latch |= 2;
	}
	if (spaceship_collision(helper, &rect))
	{
		starshp1_collision_latch |= 4;
	}
	if (spaceship_collision(helper, &machine->screen[0].visarea))
	{
		starshp1_collision_latch |= 8;
	}
}
