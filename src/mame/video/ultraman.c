#include "driver.h"
#include "video/konicdev.h"

static int sprite_colorbase, zoom_colorbase[3];
static int bank0,bank1,bank2;


/***************************************************************************

  Callbacks for the K051960

***************************************************************************/

void ultraman_sprite_callback(running_machine *machine, int *code,int *color,int *priority,int *shadow)
{
	*priority = (*color & 0x80) >> 7;
	*color = sprite_colorbase + ((*color & 0x7e) >> 1);
	*shadow = 0;
}


/***************************************************************************

  Callbacks for the K051316

***************************************************************************/

void ultraman_zoom_callback_0(running_machine *machine, int *code,int *color,int *flags)
{
	*code |= ((*color & 0x07) << 8) | (bank0 << 11);
	*color = zoom_colorbase[0] + ((*color & 0xf8) >> 3);
}

void ultraman_zoom_callback_1(running_machine *machine, int *code,int *color,int *flags)
{
	*code |= ((*color & 0x07) << 8) | (bank1 << 11);
	*color = zoom_colorbase[1] + ((*color & 0xf8) >> 3);
}

void ultraman_zoom_callback_2(running_machine *machine, int *code,int *color,int *flags)
{
	*code |= ((*color & 0x07) << 8) | (bank2 << 11);
	*color = zoom_colorbase[2] + ((*color & 0xf8) >> 3);
}



/***************************************************************************

    Start the video hardware emulation.

***************************************************************************/

VIDEO_START( ultraman )
{
	sprite_colorbase = 192;
	zoom_colorbase[0] = 0;
	zoom_colorbase[1] = 64;
	zoom_colorbase[2] = 128;
}



/***************************************************************************

  Memory handlers

***************************************************************************/

WRITE16_HANDLER( ultraman_gfxctrl_w )
{
	const device_config *k051316_1 = devtag_get_device(space->machine, "k051316_1");
	const device_config *k051316_2 = devtag_get_device(space->machine, "k051316_2");
	const device_config *k051316_3 = devtag_get_device(space->machine, "k051316_3");

	if (ACCESSING_BITS_0_7)
	{
		/*  bit 0: enable wraparound for scr #1
            bit 1: msb of code for scr #1
            bit 2: enable wraparound for scr #2
            bit 3: msb of code for scr #2
            bit 4: enable wraparound for scr #3
            bit 5: msb of code for scr #3
            bit 6: coin counter 1
            bit 7: coin counter 2 */

		k051316_wraparound_enable(k051316_1, data & 0x01);

		if (bank0 != ((data & 0x02) >> 1))
		{
			bank0 = (data & 0x02) >> 1;
			tilemap_mark_all_tiles_dirty_all(space->machine);	/* should mark only zoom0 */
		}

		k051316_wraparound_enable(k051316_2, data & 0x04);

		if (bank1 != ((data & 0x08) >> 3))
		{
			bank1 = (data & 0x08) >> 3;
			tilemap_mark_all_tiles_dirty_all(space->machine);	/* should mark only zoom1 */
		}

		k051316_wraparound_enable(k051316_3, data & 0x10);

		if (bank2 != ((data & 0x20) >> 5))
		{
			bank2 = (data & 0x20) >> 5;
			tilemap_mark_all_tiles_dirty_all(space->machine);	/* should mark only zoom2 */
		}

		coin_counter_w(space->machine, 0, data & 0x40);
		coin_counter_w(space->machine, 1, data & 0x80);
	}
}



/***************************************************************************

    Display Refresh

***************************************************************************/

VIDEO_UPDATE( ultraman )
{
	const device_config *k051316_1 = devtag_get_device(screen->machine, "k051316_1");
	const device_config *k051316_2 = devtag_get_device(screen->machine, "k051316_2");
	const device_config *k051316_3 = devtag_get_device(screen->machine, "k051316_3");
	const device_config *k051960 = devtag_get_device(screen->machine, "k051960");

	k051316_zoom_draw(k051316_3, bitmap, cliprect, 0, 0);
	k051316_zoom_draw(k051316_2, bitmap, cliprect, 0, 0);
	k051960_sprites_draw(k051960, bitmap, cliprect, 0, 0);
	k051316_zoom_draw(k051316_1, bitmap, cliprect, 0, 0);
	k051960_sprites_draw(k051960, bitmap, cliprect, 1, 1);
	return 0;
}
