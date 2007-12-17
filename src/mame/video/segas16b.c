/***************************************************************************

    Sega System 16B hardware

***************************************************************************/

#include "driver.h"
#include "segaic16.h"
#include "includes/system16.h"



/*************************************
 *
 *  Video startup
 *
 *************************************/

static void video_start_common(int type)
{
	/* compute palette info */
	segaic16_palette_init(0x800);

	/* initialize the tile/text layers */
	segaic16_tilemap_init(0, type, 0x000, 0, 2);

	/* initialize the sprites */
	segaic16_sprites_init(0, SEGAIC16_SPRITES_16B, 0x400, 0);
}


VIDEO_START( system16b )
{
	video_start_common(SEGAIC16_TILEMAP_16B);
}


VIDEO_START( timscanr )
{
	video_start_common(SEGAIC16_TILEMAP_16B_ALT);
}



/*************************************
 *
 *  Video update
 *
 *************************************/

VIDEO_UPDATE( system16b )
{
	/* if no drawing is happening, fill with black and get out */
	if (!segaic16_display_enable)
	{
		fillbitmap(bitmap, get_black_pen(machine), cliprect);
		return 0;
	}

	/* reset priorities */
	fillbitmap(priority_bitmap, 0, cliprect);

	/* draw background opaquely first, not setting any priorities */
	segaic16_tilemap_draw(machine, 0, bitmap, cliprect, SEGAIC16_TILEMAP_BACKGROUND, 0 | TILEMAP_DRAW_OPAQUE, 0x00);
	segaic16_tilemap_draw(machine, 0, bitmap, cliprect, SEGAIC16_TILEMAP_BACKGROUND, 1 | TILEMAP_DRAW_OPAQUE, 0x00);

	/* draw background again, just to set the priorities on non-transparent pixels */
	segaic16_tilemap_draw(machine, 0, NULL, cliprect, SEGAIC16_TILEMAP_BACKGROUND, 0, 0x01);
	segaic16_tilemap_draw(machine, 0, NULL, cliprect, SEGAIC16_TILEMAP_BACKGROUND, 1, 0x02);

	/* draw foreground */
	segaic16_tilemap_draw(machine, 0, bitmap, cliprect, SEGAIC16_TILEMAP_FOREGROUND, 0, 0x02);
	segaic16_tilemap_draw(machine, 0, bitmap, cliprect, SEGAIC16_TILEMAP_FOREGROUND, 1, 0x04);

	/* text layer */
	segaic16_tilemap_draw(machine, 0, bitmap, cliprect, SEGAIC16_TILEMAP_TEXT, 0, 0x04);
	segaic16_tilemap_draw(machine, 0, bitmap, cliprect, SEGAIC16_TILEMAP_TEXT, 1, 0x08);

	/* draw the sprites */
	segaic16_sprites_draw(0, bitmap, cliprect);
	return 0;
}
