/***************************************************************************

Atari Sprint 8 video emulation

***************************************************************************/

#include "driver.h"
#include "includes/sprint8.h"

UINT8* sprint8_video_ram;
UINT8* sprint8_pos_h_ram;
UINT8* sprint8_pos_v_ram;
UINT8* sprint8_pos_d_ram;
UINT8* sprint8_team;

static tilemap_t* tilemap1;
static tilemap_t* tilemap2;

static bitmap_t* helper1;
static bitmap_t* helper2;


PALETTE_INIT( sprint8 )
{
	int i;

	/* allocate the colortable */
	machine->colortable = colortable_alloc(machine, 0x12);

	for (i = 0; i < 0x10; i++)
	{
		colortable_entry_set_value(machine->colortable, 2 * i + 0, 0x10);
		colortable_entry_set_value(machine->colortable, 2 * i + 1, i);
	}

	colortable_entry_set_value(machine->colortable, 0x20, 0x10);
	colortable_entry_set_value(machine->colortable, 0x21, 0x10);
	colortable_entry_set_value(machine->colortable, 0x22, 0x10);
	colortable_entry_set_value(machine->colortable, 0x23, 0x11);
}


static void set_pens(colortable_t *colortable)
{
	int i;

	for (i = 0; i < 0x10; i += 8)
	{
		if (*sprint8_team & 1)
		{
			colortable_palette_set_color(colortable, i + 0, MAKE_RGB(0xff, 0x00, 0x00)); /* red     */
			colortable_palette_set_color(colortable, i + 1, MAKE_RGB(0x00, 0x00, 0xff)); /* blue    */
			colortable_palette_set_color(colortable, i + 2, MAKE_RGB(0xff, 0xff, 0x00)); /* yellow  */
			colortable_palette_set_color(colortable, i + 3, MAKE_RGB(0x00, 0xff, 0x00)); /* green   */
			colortable_palette_set_color(colortable, i + 4, MAKE_RGB(0xff, 0x00, 0xff)); /* magenta */
			colortable_palette_set_color(colortable, i + 5, MAKE_RGB(0xe0, 0xc0, 0x70)); /* puce    */
			colortable_palette_set_color(colortable, i + 6, MAKE_RGB(0x00, 0xff, 0xff)); /* cyan    */
			colortable_palette_set_color(colortable, i + 7, MAKE_RGB(0xff, 0xaa, 0xaa)); /* pink    */
		}
		else
		{
			colortable_palette_set_color(colortable, i + 0, MAKE_RGB(0xff, 0x00, 0x00)); /* red     */
			colortable_palette_set_color(colortable, i + 1, MAKE_RGB(0x00, 0x00, 0xff)); /* blue    */
			colortable_palette_set_color(colortable, i + 2, MAKE_RGB(0xff, 0x00, 0x00)); /* red     */
			colortable_palette_set_color(colortable, i + 3, MAKE_RGB(0x00, 0x00, 0xff)); /* blue    */
			colortable_palette_set_color(colortable, i + 4, MAKE_RGB(0xff, 0x00, 0x00)); /* red     */
			colortable_palette_set_color(colortable, i + 5, MAKE_RGB(0x00, 0x00, 0xff)); /* blue    */
			colortable_palette_set_color(colortable, i + 6, MAKE_RGB(0xff, 0x00, 0x00)); /* red     */
			colortable_palette_set_color(colortable, i + 7, MAKE_RGB(0x00, 0x00, 0xff)); /* blue    */
		}
	}

	colortable_palette_set_color(colortable, 0x10, MAKE_RGB(0x00, 0x00, 0x00));
	colortable_palette_set_color(colortable, 0x11, MAKE_RGB(0xff, 0xff, 0xff));
}


static TILE_GET_INFO( get_tile_info1 )
{
	UINT8 code = sprint8_video_ram[tile_index];

	int color = 0;

	if ((code & 0x30) != 0x30) /* ? */
		color = 17;
	else
	{
		if ((tile_index + 1) & 0x010)
			color |= 1;

		if (code & 0x80)
			color |= 2;

		if (tile_index & 0x200)
			color |= 4;

	}

	SET_TILE_INFO(code >> 7, code, color, (code & 0x40) ? (TILE_FLIPX | TILE_FLIPY) : 0);
}


static TILE_GET_INFO( get_tile_info2 )
{
	UINT8 code = sprint8_video_ram[tile_index];

	int color = 0;

	if ((code & 0x38) != 0x28)
		color = 16;
	else
		color = 17;

	SET_TILE_INFO(code >> 7, code, color, (code & 0x40) ? (TILE_FLIPX | TILE_FLIPY) : 0);
}


WRITE8_HANDLER( sprint8_video_ram_w )
{
	sprint8_video_ram[offset] = data;
	tilemap_mark_tile_dirty(tilemap1, offset);
	tilemap_mark_tile_dirty(tilemap2, offset);
}


VIDEO_START( sprint8 )
{
	helper1 = video_screen_auto_bitmap_alloc(machine->primary_screen);
	helper2 = video_screen_auto_bitmap_alloc(machine->primary_screen);

	tilemap1 = tilemap_create(machine, get_tile_info1, tilemap_scan_rows, 16, 8, 32, 32);
	tilemap2 = tilemap_create(machine, get_tile_info2, tilemap_scan_rows, 16, 8, 32, 32);

	tilemap_set_scrolly(tilemap1, 0, +24);
	tilemap_set_scrolly(tilemap2, 0, +24);
}


static void draw_sprites(running_machine *machine, bitmap_t* bitmap, const rectangle *cliprect)
{
	int i;

	for (i = 0; i < 16; i++)
	{
		UINT8 code = sprint8_pos_d_ram[i];

		int x = sprint8_pos_h_ram[i];
		int y = sprint8_pos_v_ram[i];

		if (code & 0x80)
			x |= 0x100;

		drawgfx_transpen(bitmap, cliprect, machine->gfx[2],
			code ^ 7,
			i,
			!(code & 0x10), !(code & 0x08),
			496 - x, y - 31, 0);
	}
}


static TIMER_CALLBACK( sprint8_collision_callback )
{
	sprint8_set_collision(machine, param);
}


VIDEO_UPDATE( sprint8 )
{
	set_pens(screen->machine->colortable);
	tilemap_draw(bitmap, cliprect, tilemap1, 0, 0);
	draw_sprites(screen->machine, bitmap, cliprect);
	return 0;
}


VIDEO_EOF( sprint8 )
{
	int x;
	int y;
	const rectangle *visarea = video_screen_get_visible_area(machine->primary_screen);

	tilemap_draw(helper2, visarea, tilemap2, 0, 0);

	bitmap_fill(helper1, visarea, 0x20);

	draw_sprites(machine, helper1, visarea);

	for (y = visarea->min_y; y <= visarea->max_y; y++)
	{
		const UINT16* p1 = BITMAP_ADDR16(helper1, y, 0);
		const UINT16* p2 = BITMAP_ADDR16(helper2, y, 0);

		for (x = visarea->min_x; x <= visarea->max_x; x++)
			if (p1[x] != 0x20 && p2[x] == 0x23)
				timer_set(machine, video_screen_get_time_until_pos(machine->primary_screen, y + 24, x), NULL,
						  colortable_entry_get_value(machine->colortable, p1[x]),
						  sprint8_collision_callback);
	}
}
