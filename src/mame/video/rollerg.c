#include "driver.h"
#include "video/konamiic.h"


static int bg_colorbase,sprite_colorbase,zoom_colorbase;



/***************************************************************************

  Callbacks for the K053245

***************************************************************************/

static void sprite_callback(int *code,int *color,int *priority_mask)
{
#if 0
if (input_code_pressed(KEYCODE_Q) && (*color & 0x80)) *color = rand();
if (input_code_pressed(KEYCODE_W) && (*color & 0x40)) *color = rand();
if (input_code_pressed(KEYCODE_E) && (*color & 0x20)) *color = rand();
if (input_code_pressed(KEYCODE_R) && (*color & 0x10)) *color = rand();
#endif
	*priority_mask = (*color & 0x10) ? 0 : 0x02;
	*color = sprite_colorbase + (*color & 0x0f);
}


/***************************************************************************

  Callbacks for the K051316

***************************************************************************/

static void zoom_callback(int *code,int *color,int *flags)
{
	*flags = TILE_FLIPYX((*color & 0xc0) >> 6);
	*code |= ((*color & 0x0f) << 8);
	*color = zoom_colorbase + ((*color & 0x30) >> 4);
}



/***************************************************************************

    Start the video hardware emulation.

***************************************************************************/

VIDEO_START( rollerg )
{
	bg_colorbase = 16;
	sprite_colorbase = 16;
	zoom_colorbase = 0;

	K053245_vh_start(machine,0,"gfx1",NORMAL_PLANE_ORDER,sprite_callback);
	K051316_vh_start_0(machine,"gfx2",4,FALSE,0,zoom_callback);

	K051316_set_offset(0, 22, 1);
}



/***************************************************************************

  Display refresh

***************************************************************************/

VIDEO_UPDATE( rollerg )
{
	bitmap_fill(screen->machine->priority_bitmap,cliprect,0);
	bitmap_fill(bitmap,cliprect,16 * bg_colorbase);
	K051316_zoom_draw_0(bitmap,cliprect,0,1);
	K053245_sprites_draw(screen->machine,0,bitmap,cliprect);
	return 0;
}
