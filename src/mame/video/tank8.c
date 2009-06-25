/***************************************************************************

Atari Tank 8 video emulation

***************************************************************************/

#include "driver.h"
#include "tank8.h"


UINT8 *tank8_video_ram;
UINT8 *tank8_pos_h_ram;
UINT8 *tank8_pos_v_ram;
UINT8 *tank8_pos_d_ram;
UINT8 *tank8_team;

static tilemap *tank8_tilemap;

static bitmap_t *helper1;
static bitmap_t *helper2;
static bitmap_t *helper3;



PALETTE_INIT( tank8 )
{
	int i;

	/* allocate the colortable */
	machine->colortable = colortable_alloc(machine, 0x0a);

	colortable_palette_set_color(machine->colortable, 8, MAKE_RGB(0x00, 0x00, 0x00));
	colortable_palette_set_color(machine->colortable, 9, MAKE_RGB(0xff, 0xff, 0xff));

	for (i = 0; i < 8; i++)
	{
		colortable_entry_set_value(machine->colortable, 2 * i + 0, 8);
		colortable_entry_set_value(machine->colortable, 2 * i + 1, i);
	}

	/* walls */
	colortable_entry_set_value(machine->colortable, 0x10, 8);
	colortable_entry_set_value(machine->colortable, 0x11, 9);

	/* mines */
	colortable_entry_set_value(machine->colortable, 0x12, 8);
	colortable_entry_set_value(machine->colortable, 0x13, 9);
}


static void set_pens(colortable_t *colortable)
{
	if (*tank8_team & 0x01)
	{
		colortable_palette_set_color(colortable, 0, MAKE_RGB(0xff, 0x00, 0x00)); /* red     */
		colortable_palette_set_color(colortable, 1, MAKE_RGB(0x00, 0x00, 0xff)); /* blue    */
		colortable_palette_set_color(colortable, 2, MAKE_RGB(0xff, 0xff, 0x00)); /* yellow  */
		colortable_palette_set_color(colortable, 3, MAKE_RGB(0x00, 0xff, 0x00)); /* green   */
		colortable_palette_set_color(colortable, 4, MAKE_RGB(0xff, 0x00, 0xff)); /* magenta */
		colortable_palette_set_color(colortable, 5, MAKE_RGB(0xe0, 0xc0, 0x70)); /* puce    */
		colortable_palette_set_color(colortable, 6, MAKE_RGB(0x00, 0xff, 0xff)); /* cyan    */
		colortable_palette_set_color(colortable, 7, MAKE_RGB(0xff, 0xaa, 0xaa)); /* pink    */
	}
	else
	{
		colortable_palette_set_color(colortable, 0, MAKE_RGB(0xff, 0x00, 0x00)); /* red     */
		colortable_palette_set_color(colortable, 2, MAKE_RGB(0xff, 0x00, 0x00)); /* red     */
		colortable_palette_set_color(colortable, 4, MAKE_RGB(0xff, 0x00, 0x00)); /* red     */
		colortable_palette_set_color(colortable, 6, MAKE_RGB(0xff, 0x00, 0x00)); /* red     */
		colortable_palette_set_color(colortable, 1, MAKE_RGB(0x00, 0x00, 0xff)); /* blue    */
		colortable_palette_set_color(colortable, 3, MAKE_RGB(0x00, 0x00, 0xff)); /* blue    */
		colortable_palette_set_color(colortable, 5, MAKE_RGB(0x00, 0x00, 0xff)); /* blue    */
		colortable_palette_set_color(colortable, 7, MAKE_RGB(0x00, 0x00, 0xff)); /* blue    */
	}
}


WRITE8_HANDLER( tank8_video_ram_w )
{
	tank8_video_ram[offset] = data;
	tilemap_mark_tile_dirty(tank8_tilemap, offset);
}



static TILE_GET_INFO( tank8_get_tile_info )
{
	UINT8 code = tank8_video_ram[tile_index];

	int color = 0;

	if ((code & 0x38) == 0x28)
	{
		if ((code & 7) != 3)
			color = 8; /* walls */
		else
			color = 9; /* mines */
	}
	else
	{
		if (tile_index & 0x010)
			color |= 1;

		if (code & 0x80)
			color |= 2;

		if (tile_index & 0x200)
			color |= 4;
	}

	SET_TILE_INFO(code >> 7, code, color, (code & 0x40) ? (TILE_FLIPX | TILE_FLIPY) : 0);
}



VIDEO_START( tank8 )
{
	helper1 = video_screen_auto_bitmap_alloc(machine->primary_screen);
	helper2 = video_screen_auto_bitmap_alloc(machine->primary_screen);
	helper3 = video_screen_auto_bitmap_alloc(machine->primary_screen);

	tank8_tilemap = tilemap_create(machine, tank8_get_tile_info, tilemap_scan_rows, 16, 16, 32, 32);

	/* VBLANK starts on scanline #256 and ends on scanline #24 */

	tilemap_set_scrolly(tank8_tilemap, 0, 2 * 24);
}


static int get_x_pos(int n)
{
	return 498 - tank8_pos_h_ram[n] - 2 * (tank8_pos_d_ram[n] & 128); /* ? */
}


static int get_y_pos(int n)
{
	return 2 * tank8_pos_v_ram[n] - 62;
}


static void draw_sprites(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect)
{
	int i;

	for (i = 0; i < 8; i++)
	{
		UINT8 code = ~tank8_pos_d_ram[i];

		int x = get_x_pos(i);
		int y = get_y_pos(i);

		drawgfx_transpen(bitmap, cliprect, machine->gfx[(code & 0x04) ? 2 : 3],
			code & 0x03,
			i,
			code & 0x10,
			code & 0x08,
			x,
			y, 0);
	}
}


static void draw_bullets(bitmap_t *bitmap, const rectangle *cliprect)
{
	int i;

	for (i = 0; i < 8; i++)
	{
		rectangle rect;

		int x = get_x_pos(8 + i);
		int y = get_y_pos(8 + i);

		x -= 4; /* ? */

		rect.min_x = x;
		rect.min_y = y;
		rect.max_x = rect.min_x + 3;
		rect.max_y = rect.min_y + 4;

		if (rect.min_x < cliprect->min_x)
			rect.min_x = cliprect->min_x;
		if (rect.min_y < cliprect->min_y)
			rect.min_y = cliprect->min_y;
		if (rect.max_x > cliprect->max_x)
			rect.max_x = cliprect->max_x;
		if (rect.max_y > cliprect->max_y)
			rect.max_y = cliprect->max_y;

		bitmap_fill(bitmap, &rect, (i << 1) | 0x01);
	}
}


static TIMER_CALLBACK( tank8_collision_callback )
{
	tank8_set_collision(machine, param);
}


VIDEO_UPDATE( tank8 )
{
	set_pens(screen->machine->colortable);
	tilemap_draw(bitmap, cliprect, tank8_tilemap, 0, 0);

	draw_sprites(screen->machine, bitmap, cliprect);
	draw_bullets(bitmap, cliprect);
	return 0;
}


VIDEO_EOF( tank8 )
{
	int x;
	int y;
	const rectangle *visarea = video_screen_get_visible_area(machine->primary_screen);

	tilemap_draw(helper1, visarea, tank8_tilemap, 0, 0);

	bitmap_fill(helper2, visarea, 8);
	bitmap_fill(helper3, visarea, 8);

	draw_sprites(machine, helper2, visarea);
	draw_bullets(helper3, visarea);

	for (y = visarea->min_y; y <= visarea->max_y; y++)
	{
		int state = 0;

		const UINT16* p1 = BITMAP_ADDR16(helper1, y, 0);
		const UINT16* p2 = BITMAP_ADDR16(helper2, y, 0);
		const UINT16* p3 = BITMAP_ADDR16(helper3, y, 0);

		if (y % 2 != video_screen_get_frame_number(machine->primary_screen) % 2)
			continue; /* video display is interlaced */

		for (x = visarea->min_x; x <= visarea->max_x; x++)
		{
			UINT8 index;

			/* neither wall nor mine */
			if ((p1[x] != 0x11) && (p1[x] != 0x13))
			{
				state = 0;
				continue;
			}

			/* neither tank nor bullet */
			if ((p2[x] == 8) && (p3[x] == 8))
			{
				state = 0;
				continue;
			}

			/* bullets cannot hit mines */
			if ((p3[x] != 8) && (p1[x] == 0x13))
			{
				state = 0;
				continue;
			}

			if (state)
				continue;

			if (p3[x] != 8)
			{
				index = ((p3[x] & ~0x01) >> 1) | 0x18;

				if (1)
					index |= 0x20;

				if (0)
					index |= 0x40;

				if (1)
					index |= 0x80;
			}
			else
			{
				int sprite_num = (p2[x] & ~0x01) >> 1;
				index = sprite_num | 0x10;

				if (p1[x] == 0x11)
					index |= 0x20;

				if (y - get_y_pos(sprite_num) >= 8)
					index |= 0x40; /* collision on bottom side */

				if (x - get_x_pos(sprite_num) >= 8)
					index |= 0x80; /* collision on right side */
			}

			timer_set(machine, video_screen_get_time_until_pos(machine->primary_screen, y, x), NULL, index, tank8_collision_callback);

			state = 1;
		}
	}
}
