/***************************************************************************

Atari Fire Truck + Super Bug + Monte Carlo video emulation

***************************************************************************/

#include "driver.h"
#include "firetrk.h"

UINT8* firetrk_alpha_num_ram;
UINT8* firetrk_playfield_ram;

int firetrk_skid[2];
int firetrk_crash[2];

static mame_bitmap *helper1;
static mame_bitmap *helper2;

static int blink;
static int flash;
static int drone_hpos;
static int drone_vpos;

static const rectangle playfield_window = { 0x02A, 0x115, 0x000, 0x0FF };

struct sprite_data
{
	int layout;
	int number;
	int x;
	int y;
	int flipx;
	int flipy;
	int color;
};

static struct sprite_data car[2];

static tilemap* tilemap1; /* for screen display */
static tilemap* tilemap2; /* for collision detection */



INLINE int arrow_code(int c)
{
	if (GAME_IS_FIRETRUCK)
	{
		return (c & 0x3F) >= 0x4 && (c & 0x3F) <= 0xB;
	}
	if (GAME_IS_SUPERBUG)
	{
		return (c & 0x3F) >= 0x8 && (c & 0x3F) <= 0xF;
	}

	return 0;
}


void firetrk_set_flash(int flag)
{
	tilemap_mark_all_tiles_dirty(tilemap1);

	if (GAME_IS_FIRETRUCK || GAME_IS_SUPERBUG)
	{
		if (flag)
		{
			car[0].color = 1;
			car[1].color = 1;
		}
		else
		{
			car[0].color = 0;
			car[1].color = 0;
		}
	}

	flash = flag;
}


void firetrk_set_blink(int flag)
{
	int offset;

	for (offset = 0; offset < 0x100; offset++)
	{
		if (arrow_code(firetrk_playfield_ram[offset]))
		{
			tilemap_mark_tile_dirty(tilemap1, offset);
		}
	}

	blink = flag;
}


static TILE_GET_INFO( get_tile_info1 )
{
	UINT8 code = firetrk_playfield_ram[tile_index];

	int color = code >> 6;

	if (blink && arrow_code(code))
	{
		color = 0;
	}
	if (flash)
	{
		color |= 4;
	}

	SET_TILE_INFO(1, code & 0x3f, color, 0);
}


static TILE_GET_INFO( get_tile_info2 )
{
	UINT8 code = firetrk_playfield_ram[tile_index];

	int color = 0;

	/* palette 1 for crash and palette 2 for skid */

	if (GAME_IS_FIRETRUCK)
	{
		if ((code & 0x30) != 0x00 || (code & 0x0c) == 0x00)
		{
			color = 1;   /* palette 0, 1 */
		}
		if ((code & 0x3c) == 0x0c)
		{
			color = 2;   /* palette 0, 2 */
		}
	}

	if (GAME_IS_SUPERBUG)
	{
		if ((code & 0x30) != 0x00)
		{
			color = 1;   /* palette 0, 1 */
		}
		if ((code & 0x38) == 0x00)
		{
			color = 2;   /* palette 0, 2 */
		}
	}

	if (GAME_IS_MONTECARLO)
	{
		if ((code & 0xc0) == 0x40 || (code & 0xc0) == 0x80)
		{
			color = 2;   /* palette 2, 1 */
		}
		if ((code & 0xc0) == 0xc0)
		{
			color = 1;   /* palette 2, 0 */
		}
		if ((code & 0xc0) == 0x00)
		{
			color = 3;   /* palette 2, 2 */
		}
		if ((code & 0x30) == 0x30)
		{
			color = 0;   /* palette 0, 0 */
		}
	}

	SET_TILE_INFO(2, code & 0x3f, color, 0);
}


WRITE8_HANDLER( firetrk_vert_w )
{
	tilemap_set_scrolly(tilemap1, 0, data);
	tilemap_set_scrolly(tilemap2, 0, data);
}


WRITE8_HANDLER( firetrk_horz_w )
{
	tilemap_set_scrollx(tilemap1, 0, data - 37);
	tilemap_set_scrollx(tilemap2, 0, data - 37);
}


WRITE8_HANDLER( firetrk_drone_hpos_w )
{
	drone_hpos = data;
}


WRITE8_HANDLER( firetrk_drone_vpos_w )
{
	drone_vpos = data;
}


WRITE8_HANDLER( firetrk_car_rot_w )
{
	if (GAME_IS_FIRETRUCK)
	{
		car[0].number = data & 0x03;

		if (data & 0x10) /* swap xy */
		{
			car[0].layout = 4;
		}
		else
		{
			car[0].layout = 3;
		}

		car[0].flipx = data & 0x04;
		car[0].flipy = data & 0x08;
	}

	if (GAME_IS_SUPERBUG)
	{
		car[0].number = (data & 0x03) ^ 3;

		if (data & 0x10) /* swap xy */
		{
			car[0].layout = 4;
		}
		else
		{
			car[0].layout = 3;
		}

		car[0].flipx = data & 0x04;
		car[0].flipy = data & 0x08;
	}

	if (GAME_IS_MONTECARLO)
	{
		car[0].number = data & 0x07;

		if (data & 0x80)
		{
			car[1].color |= 2;
		}
		else
		{
			car[1].color &= ~2;
		}

		car[0].flipx = data & 0x10;
		car[0].flipy = data & 0x08;
	}
}


WRITE8_HANDLER( firetrk_drone_rot_w )
{
	car[1].number = data & 0x07;

	if (GAME_IS_FIRETRUCK)
	{
		car[1].flipx = data & 0x08;
		car[1].flipy = data & 0x10;
	}

	if (GAME_IS_MONTECARLO)
	{
		car[1].flipx = data & 0x10;
		car[1].flipy = data & 0x08;

		if (data & 0x80)
		{
			car[1].color |= 1;
		}
		else
		{
			car[1].color &= ~1;
		}
	}
}


WRITE8_HANDLER( firetrk_playfield_w )
{
	firetrk_playfield_ram[offset] = data;
	tilemap_mark_tile_dirty(tilemap1, offset);
	tilemap_mark_tile_dirty(tilemap2, offset);
}


VIDEO_START( firetrk )
{
	helper1 = auto_bitmap_alloc(machine->screen[0].width, machine->screen[0].height, machine->screen[0].format);
	helper2 = auto_bitmap_alloc(machine->screen[0].width, machine->screen[0].height, machine->screen[0].format);

	tilemap1 = tilemap_create(get_tile_info1, tilemap_scan_rows, TILEMAP_TYPE_PEN, 16, 16, 16, 16);
	tilemap2 = tilemap_create(get_tile_info2, tilemap_scan_rows, TILEMAP_TYPE_PEN, 16, 16, 16, 16);

	memset(&car[0], 0, sizeof (struct sprite_data));
	memset(&car[1], 0, sizeof (struct sprite_data));

	if (GAME_IS_FIRETRUCK)
	{
		car[0].layout = 3;
		car[1].layout = 5;
	}
	if (GAME_IS_SUPERBUG)
	{
		car[0].layout = 3;
		car[1].layout = 0;
	}
	if (GAME_IS_MONTECARLO)
	{
		car[0].layout = 3;
		car[1].layout = 4;
	}
}


static void calc_car_positions(void)
{
	car[0].x = 144;
	car[0].y = 104;

	if (GAME_IS_FIRETRUCK)
	{
		car[1].x = car[1].flipx ? drone_hpos - 63 : 192 - drone_hpos;
		car[1].y = car[1].flipy ? drone_vpos - 63 : 192 - drone_vpos;

		car[1].x += 36;
	}

	if (GAME_IS_MONTECARLO)
	{
		car[1].x = car[1].flipx ? drone_hpos - 31 : 224 - drone_hpos;
		car[1].y = car[1].flipy ? drone_vpos - 31 : 224 - drone_vpos;

		car[1].x += 34;
	}
}


static void draw_text(running_machine *machine, mame_bitmap* bitmap, const rectangle* cliprect)
{
	const UINT8* p = firetrk_alpha_num_ram;

	int i;

	for (i = 0; i < 2; i++)
	{
		int x = 0;
		int y = 0;

		if (GAME_IS_SUPERBUG || GAME_IS_FIRETRUCK)
		{
			x = (i == 0) ? 296 : 8;
		}
		if (GAME_IS_MONTECARLO)
		{
			x = (i == 0) ? 24 : 16;
		}

		for (y = 0; y < 256; y += machine->gfx[0]->width)
		{
			drawgfx(bitmap, machine->gfx[0], *p++, 0, 0, 0,
				x, y, cliprect, TRANSPARENCY_NONE, 0);
		}
	}
}


VIDEO_UPDATE( firetrk )
{
	int i;

	tilemap_draw(bitmap, &playfield_window, tilemap1, 0, 0);

	calc_car_positions();

	for (i = 1; i >= 0; i--)
	{
		if (GAME_IS_SUPERBUG && i == 1)
		{
			continue;
		}

		drawgfx(bitmap,
			machine->gfx[car[i].layout],
			car[i].number,
			car[i].color,
			car[i].flipx,
			car[i].flipy,
			car[i].x,
			car[i].y,
			&playfield_window,
			TRANSPARENCY_PEN, 0);
	}

	draw_text(machine, bitmap, cliprect);
	return 0;
}


VIDEO_EOF( firetrk )
{
	int i;

	tilemap_draw(helper1, &playfield_window, tilemap2, 0, 0);

	calc_car_positions();

	for (i = 1; i >= 0; i--)
	{
		int width = machine->gfx[car[i].layout]->width;
		int height = machine->gfx[car[i].layout]->height;

		int x;
		int y;

		if (GAME_IS_SUPERBUG && i == 1)
		{
			continue;
		}

		drawgfx(helper2,
			machine->gfx[car[i].layout],
			car[i].number,
			0,
			car[i].flipx,
			car[i].flipy,
			car[i].x,
			car[i].y,
			&playfield_window,
			TRANSPARENCY_NONE, 0);

		for (y = car[i].y; y < car[i].y + height; y++)
		{
			for (x = car[i].x; x < car[i].x + width; x++)
			{
				pen_t a;
				pen_t b;

				if (x < playfield_window.min_x)
					continue;
				if (x > playfield_window.max_x)
					continue;
				if (y < playfield_window.min_y)
					continue;
				if (y > playfield_window.max_y)
					continue;

				a = *BITMAP_ADDR16(helper1, y, x);
				b = *BITMAP_ADDR16(helper2, y, x);

				if (b != 0 && a == 1)
				{
					firetrk_crash[i] = 1;
				}
				if (b != 0 && a == 2)
				{
					firetrk_skid[i] = 1;
				}
			}
		}
	}

	if (blink)
	{
		firetrk_set_blink(0);
	}
}
