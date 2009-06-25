/* One Shot One Kill Video Hardware */

#include "driver.h"
#include "includes/oneshot.h"

static tilemap *oneshot_bg_tilemap;
static tilemap *oneshot_mid_tilemap;
static tilemap *oneshot_fg_tilemap;

/* bg tilemap */
static TILE_GET_INFO( get_oneshot_bg_tile_info )
{
	int tileno;

	tileno = oneshot_bg_videoram[tile_index*2+1];

	SET_TILE_INFO(0,tileno,0,0);
}

WRITE16_HANDLER( oneshot_bg_videoram_w )
{
	COMBINE_DATA(&oneshot_bg_videoram[offset]);
	tilemap_mark_tile_dirty(oneshot_bg_tilemap,offset/2);
}

/* mid tilemap */
static TILE_GET_INFO( get_oneshot_mid_tile_info )
{
	int tileno;

	tileno = oneshot_mid_videoram[tile_index*2+1];

	SET_TILE_INFO(0,tileno,2,0);
}

WRITE16_HANDLER( oneshot_mid_videoram_w )
{
	COMBINE_DATA(&oneshot_mid_videoram[offset]);
	tilemap_mark_tile_dirty(oneshot_mid_tilemap,offset/2);
}


/* fg tilemap */
static TILE_GET_INFO( get_oneshot_fg_tile_info )
{
	int tileno;

	tileno = oneshot_fg_videoram[tile_index*2+1];

	SET_TILE_INFO(0,tileno,3,0);
}

WRITE16_HANDLER( oneshot_fg_videoram_w )
{
	COMBINE_DATA(&oneshot_fg_videoram[offset]);
	tilemap_mark_tile_dirty(oneshot_fg_tilemap,offset/2);
}

VIDEO_START( oneshot )
{
	oneshot_bg_tilemap = tilemap_create(machine, get_oneshot_bg_tile_info,tilemap_scan_rows, 16, 16,32,32);
	oneshot_mid_tilemap = tilemap_create(machine, get_oneshot_mid_tile_info,tilemap_scan_rows, 16, 16,32,32);
	oneshot_fg_tilemap = tilemap_create(machine, get_oneshot_fg_tile_info,tilemap_scan_rows, 16, 16,32,32);

	tilemap_set_transparent_pen(oneshot_bg_tilemap,0);
	tilemap_set_transparent_pen(oneshot_mid_tilemap,0);
	tilemap_set_transparent_pen(oneshot_fg_tilemap,0);
}

static void draw_crosshairs( running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect )
{
    int xpos,ypos;
    /* get gun raw coordonates (player 1) */
    gun_x_p1 = (input_port_read(machine, "LIGHT0_X") & 0xff) * 320 / 256;
    gun_y_p1 = (input_port_read(machine, "LIGHT0_Y") & 0xff) * 240 / 256;

    /* compute the coordonates for drawing (from routine at 0x009ab0) */
    xpos = gun_x_p1;
    ypos = gun_y_p1;

    gun_x_p1+=gun_x_shift;

    gun_y_p1 -= 0x0a;
    if (gun_y_p1 < 0)
        gun_y_p1=0;


    /* get gun raw coordonates (player 2) */
    gun_x_p2 = (input_port_read(machine, "LIGHT1_X") & 0xff) * 320 / 256;
    gun_y_p2 = (input_port_read(machine, "LIGHT1_Y") & 0xff) * 240 / 256;
    /* compute the coordonates for drawing (from routine at 0x009b6e) */
    xpos = gun_x_p2;
    ypos = gun_y_p2;

    gun_x_p2 += gun_x_shift-0x0a;
    if (gun_x_p2 < 0)
        gun_x_p2=0;
}

static void draw_sprites(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect)
{
	const UINT16 *source = oneshot_sprites;
	const UINT16 *finish = source+(0x1000/2);
	const gfx_element *gfx = machine->gfx[1];

	int xpos,ypos;

	while( source<finish )
	{
		int blockx,blocky;
		int num = source[1] & 0xffff;
		int xsize = (source[2] & 0x000f)+1;
		int ysize = (source[3] & 0x000f)+1;

		ypos = source[3] & 0xff80;
		xpos = source[2] & 0xff80;

		ypos = ypos >> 7;
		xpos = xpos >> 7;


		if (source[0] == 0x0001) break;

		xpos -= 8;
		ypos -= 6;


		for (blockx = 0; blockx<xsize;blockx++) {
			for (blocky = 0; blocky<ysize;blocky++) {


				drawgfx_transpen(
						bitmap,
						cliprect,
						gfx,
						num+(blocky*xsize)+blockx,
						1,
						0,0,
						xpos+blockx*8,ypos+blocky*8,0
						);

				drawgfx_transpen(
						bitmap,
						cliprect,
						gfx,
						num+(blocky*xsize)+blockx,
						1,
						0,0,
						xpos+blockx*8-0x200,ypos+blocky*8,0
						);
			}
		}
		source += 0x4;
	}

}

VIDEO_UPDATE( oneshot )
{
	bitmap_fill(bitmap, cliprect, get_black_pen(screen->machine));

	tilemap_set_scrollx(oneshot_mid_tilemap,0, oneshot_scroll[0]-0x1f5);
	tilemap_set_scrolly(oneshot_mid_tilemap,0, oneshot_scroll[1]);

	tilemap_draw(bitmap,cliprect,oneshot_bg_tilemap,0,0);
	tilemap_draw(bitmap,cliprect,oneshot_mid_tilemap,0,0);
	draw_sprites(screen->machine,bitmap,cliprect);
	tilemap_draw(bitmap,cliprect,oneshot_fg_tilemap,0,0);
	draw_crosshairs(screen->machine,bitmap,cliprect);
	return 0;
}

VIDEO_UPDATE( maddonna )
{
	bitmap_fill(bitmap, cliprect, get_black_pen(screen->machine));

	tilemap_set_scrolly(oneshot_mid_tilemap,0, oneshot_scroll[1]); // other registers aren't used so we don't know which layers they relate to

	tilemap_draw(bitmap,cliprect,oneshot_mid_tilemap,0,0);
	tilemap_draw(bitmap,cliprect,oneshot_fg_tilemap,0,0);
	tilemap_draw(bitmap,cliprect,oneshot_bg_tilemap,0,0);
	draw_sprites(screen->machine,bitmap,cliprect);
//  draw_crosshairs(screen->machine,bitmap,cliprect); // not a gun game

//  popmessage ("%04x %04x %04x %04x %04x %04x %04x %04x", oneshot_scroll[0],oneshot_scroll[1],oneshot_scroll[2],oneshot_scroll[3],oneshot_scroll[4],oneshot_scroll[5],oneshot_scroll[6],oneshot_scroll[7]);
	return 0;
}
