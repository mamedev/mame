/***************************************************************************

    Sega Hang On hardware

***************************************************************************/

#include "driver.h"
#include "segaic16.h"
#include "includes/system16.h"



/*************************************
 *
 *  Video startup
 *
 *************************************/

VIDEO_START( hangon )
{
	/* compute palette info */
	segaic16_palette_init(0x800);

	/* initialize the tile/text layers */
	segaic16_tilemap_init(0, SEGAIC16_TILEMAP_HANGON, 0x000, 0, 2);

	/* initialize the sprites */
	segaic16_sprites_init(0, SEGAIC16_SPRITES_HANGON, 0x400, 0);

	/* initialize the road */
	segaic16_road_init(0, SEGAIC16_ROAD_HANGON, 0x038, 0x7c0, 0x7c0, 0);
}


VIDEO_START( sharrier )
{
	/* compute palette info */
	segaic16_palette_init(0x800);

	/* initialize the tile/text layers */
	segaic16_tilemap_init(0, SEGAIC16_TILEMAP_HANGON, 0x000, 0, 2);

	/* initialize the sprites */
	segaic16_sprites_init(0, SEGAIC16_SPRITES_SHARRIER, 0x400, 0);

	/* initialize the road */
	segaic16_road_init(0, SEGAIC16_ROAD_SHARRIER, 0x038, 0x7c0, 0x7c0, 0);
}



/*************************************
 *
 *  Video update
 *
 *************************************/

VIDEO_UPDATE( hangon )
{
	/* if no drawing is happening, fill with black and get out */
	if (!segaic16_display_enable)
	{
		fillbitmap(bitmap, get_black_pen(machine), cliprect);
		return 0;
	}

	/* reset priorities */
	fillbitmap(priority_bitmap, 0, cliprect);

	/* draw the low priority road layer */
	segaic16_road_draw(0, bitmap, cliprect, SEGAIC16_ROAD_BACKGROUND);

	/* draw background */
	segaic16_tilemap_draw(machine, 0, bitmap, cliprect, SEGAIC16_TILEMAP_BACKGROUND, 0, 0x01);
	segaic16_tilemap_draw(machine, 0, bitmap, cliprect, SEGAIC16_TILEMAP_BACKGROUND, 1, 0x02);

	/* draw foreground */
	segaic16_tilemap_draw(machine, 0, bitmap, cliprect, SEGAIC16_TILEMAP_FOREGROUND, 0, 0x02);
	segaic16_tilemap_draw(machine, 0, bitmap, cliprect, SEGAIC16_TILEMAP_FOREGROUND, 1, 0x04);

	/* draw the high priority road */
	segaic16_road_draw(0, bitmap, cliprect, SEGAIC16_ROAD_FOREGROUND);

	/* text layer */
	/* note that we inflate the priority of the text layer to prevent sprites */
	/* from drawing over the high scores */
	segaic16_tilemap_draw(machine, 0, bitmap, cliprect, SEGAIC16_TILEMAP_TEXT, 0, 0x08);
	segaic16_tilemap_draw(machine, 0, bitmap, cliprect, SEGAIC16_TILEMAP_TEXT, 1, 0x08);

	/* draw the sprites */
	segaic16_sprites_draw(0, bitmap, cliprect);
	return 0;
}
