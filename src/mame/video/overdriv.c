#include "driver.h"
#include "video/konicdev.h"


static int zoom_colorbase[2],road_colorbase[2],sprite_colorbase;


/***************************************************************************

  Callbacks for the K053247

***************************************************************************/

void overdriv_sprite_callback(running_machine *machine, int *code,int *color,int *priority_mask)
{
	int pri = (*color & 0xffe0) >> 5;	/* ??????? */
	if (pri) *priority_mask = 0x02;
	else     *priority_mask = 0x00;

	*color = sprite_colorbase + (*color & 0x001f);
}


/***************************************************************************

  Callbacks for the K051316

***************************************************************************/

void overdriv_zoom_callback_0(running_machine *machine, int *code,int *color,int *flags)
{
	*flags = (*color & 0x40) ? TILE_FLIPX : 0;
	*code |= ((*color & 0x03) << 8);
	*color = zoom_colorbase[0] + ((*color & 0x3c) >> 2);
}

void overdriv_zoom_callback_1(running_machine *machine, int *code,int *color,int *flags)
{
	*flags = (*color & 0x40) ? TILE_FLIPX : 0;
	*code |= ((*color & 0x03) << 8);
	*color = zoom_colorbase[1] + ((*color & 0x3c) >> 2);
}



/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

VIDEO_START( overdriv )
{
//  const device_config *k051316_1 = devtag_get_device(machine, "k051316_1");
//  const device_config *k051316_2 = devtag_get_device(machine, "k051316_2");

//  K053251_vh_start(machine);
//  K051316_vh_start_0(machine,"gfx2",4,TRUE,0,zoom_callback_0);
//  K051316_vh_start_1(machine,"gfx3",4,FALSE,0,zoom_callback_1);
//  K053247_vh_start(machine,"gfx1",77,22,NORMAL_PLANE_ORDER,overdriv_sprite_callback);

//  k051316_wraparound_enable(k051316_1, 1);
//  k051316_set_offset(k051316_1, 14, -1);
//  k051316_set_offset(k051316_2, 15, 0);
}



/***************************************************************************

  Display refresh

***************************************************************************/

VIDEO_UPDATE( overdriv )
{
	const device_config *k051316_1 = devtag_get_device(screen->machine, "k051316_1");
	const device_config *k051316_2 = devtag_get_device(screen->machine, "k051316_2");
	const device_config *k053246 = devtag_get_device(screen->machine, "k053246");
	const device_config *k053251 = devtag_get_device(screen->machine, "k053251");

	sprite_colorbase  = k053251_get_palette_index(k053251, K053251_CI0);
	road_colorbase[1] = k053251_get_palette_index(k053251, K053251_CI1);
	road_colorbase[0] = k053251_get_palette_index(k053251, K053251_CI2);
	zoom_colorbase[1] = k053251_get_palette_index(k053251, K053251_CI3);
	zoom_colorbase[0] = k053251_get_palette_index(k053251, K053251_CI4);

	bitmap_fill(screen->machine->priority_bitmap, cliprect, 0);

	k051316_zoom_draw(k051316_1, bitmap, cliprect, 0, 0);
	k051316_zoom_draw(k051316_2, bitmap, cliprect, 0, 1);

	k053247_sprites_draw(k053246, bitmap,cliprect);
	return 0;
}
