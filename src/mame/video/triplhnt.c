/***************************************************************************

Atari Triple Hunt video emulation

***************************************************************************/

#include "driver.h"
#include "includes/triplhnt.h"


UINT8* triplhnt_playfield_ram;
UINT8* triplhnt_hpos_ram;
UINT8* triplhnt_vpos_ram;
UINT8* triplhnt_code_ram;
UINT8* triplhnt_orga_ram;

int triplhnt_sprite_zoom;
int triplhnt_sprite_bank;

static bitmap_t* helper;
static tilemap* bg_tilemap;


static TILE_GET_INFO( get_tile_info )
{
	int code = triplhnt_playfield_ram[tile_index] & 0x3f;

	SET_TILE_INFO(2, code, code == 0x3f ? 1 : 0, 0);
}


VIDEO_START( triplhnt )
{
	helper = video_screen_auto_bitmap_alloc(machine->primary_screen);

	bg_tilemap = tilemap_create(machine, get_tile_info, tilemap_scan_rows, 16, 16, 16, 16);
}


static TIMER_CALLBACK( triplhnt_hit_callback )
{
	triplhnt_set_collision(machine, param);
}


static void draw_sprites(running_machine *machine, bitmap_t* bitmap, const rectangle* cliprect)
{
	int i;

	int hit_line = 999;
	int hit_code = 999;

	for (i = 0; i < 16; i++)
	{
		rectangle rect;

		int j = (triplhnt_orga_ram[i] & 15) ^ 15;

		/* software sorts sprites by x and stores order in orga RAM */

		int hpos = triplhnt_hpos_ram[j] ^ 255;
		int vpos = triplhnt_vpos_ram[j] ^ 255;
		int code = triplhnt_code_ram[j] ^ 255;

		if (hpos == 255)
			continue;

		/* sprite placement might be wrong */

		if (triplhnt_sprite_zoom)
		{
			rect.min_x = hpos - 16;
			rect.min_y = 196 - vpos;
			rect.max_x = rect.min_x + 63;
			rect.max_y = rect.min_y + 63;
		}
		else
		{
			rect.min_x = hpos - 16;
			rect.min_y = 224 - vpos;
			rect.max_x = rect.min_x + 31;
			rect.max_y = rect.min_y + 31;
		}

		/* render sprite to auxiliary bitmap */

		drawgfx_opaque(helper, cliprect, machine->gfx[triplhnt_sprite_zoom],
			2 * code + triplhnt_sprite_bank, 0, code & 8, 0,
			rect.min_x, rect.min_y);

		if (rect.min_x < cliprect->min_x)
			rect.min_x = cliprect->min_x;
		if (rect.min_y < cliprect->min_y)
			rect.min_y = cliprect->min_y;
		if (rect.max_x > cliprect->max_x)
			rect.max_x = cliprect->max_x;
		if (rect.max_y > cliprect->max_y)
			rect.max_y = cliprect->max_y;

		/* check for collisions and copy sprite */

		{
			int x;
			int y;

			for (x = rect.min_x; x <= rect.max_x; x++)
			{
				for (y = rect.min_y; y <= rect.max_y; y++)
				{
					pen_t a = *BITMAP_ADDR16(helper, y, x);
					pen_t b = *BITMAP_ADDR16(bitmap, y, x);

					if (a == 2 && b == 7)
					{
						hit_code = j;
						hit_line = y;
					}

					if (a != 1)
						*BITMAP_ADDR16(bitmap, y, x) = a;
				}
			}
		}
	}

	if (hit_line != 999 && hit_code != 999)
		timer_set(machine, video_screen_get_time_until_pos(machine->primary_screen, hit_line, 0), NULL, hit_code, triplhnt_hit_callback);
}


VIDEO_UPDATE( triplhnt )
{
	const device_config *discrete = devtag_get_device(screen->machine, "discrete");

	tilemap_mark_all_tiles_dirty(bg_tilemap);

	tilemap_draw(bitmap, cliprect, bg_tilemap, 0, 0);

	draw_sprites(screen->machine, bitmap, cliprect);

	discrete_sound_w(discrete, TRIPLHNT_BEAR_ROAR_DATA, triplhnt_playfield_ram[0xfa] & 15);
	discrete_sound_w(discrete, TRIPLHNT_SHOT_DATA, triplhnt_playfield_ram[0xfc] & 15);
	return 0;
}
