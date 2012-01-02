/***************************************************************************

    Sega System 16A hardware

***************************************************************************/

#include "emu.h"
#include "video/segaic16.h"
#include "includes/segas16.h"



/*************************************
 *
 *  Video startup
 *
 *************************************/

VIDEO_START( system16a )
{
	/* compute palette info */
	segaic16_palette_init(0x800);

	/* initialize the tile/text layers */
	segaic16_tilemap_init(machine, 0, SEGAIC16_TILEMAP_16A, 0x000, 0, 1);
}



/*************************************
 *
 *  Video update
 *
 *************************************/

SCREEN_UPDATE( system16a )
{
	/* if no drawing is happening, fill with black and get out */
	if (!segaic16_display_enable)
	{
		bitmap.fill(get_black_pen(screen.machine()), cliprect);
		return 0;
	}

	/* reset priorities */
	screen.machine().priority_bitmap.fill(0, cliprect);

	/* draw background opaquely first, not setting any priorities */
	segaic16_tilemap_draw(screen, bitmap, cliprect, 0, SEGAIC16_TILEMAP_BACKGROUND, 0 | TILEMAP_DRAW_OPAQUE, 0x00);
	segaic16_tilemap_draw(screen, bitmap, cliprect, 0, SEGAIC16_TILEMAP_BACKGROUND, 1 | TILEMAP_DRAW_OPAQUE, 0x00);

	/* draw background again, just to set the priorities on non-transparent pixels */
	bitmap_t dummy_bitmap;
	segaic16_tilemap_draw(screen, dummy_bitmap, cliprect, 0, SEGAIC16_TILEMAP_BACKGROUND, 0, 0x01);
	segaic16_tilemap_draw(screen, dummy_bitmap, cliprect, 0, SEGAIC16_TILEMAP_BACKGROUND, 1, 0x02);

	/* draw foreground */
	segaic16_tilemap_draw(screen, bitmap, cliprect, 0, SEGAIC16_TILEMAP_FOREGROUND, 0, 0x02);
	segaic16_tilemap_draw(screen, bitmap, cliprect, 0, SEGAIC16_TILEMAP_FOREGROUND, 1, 0x04);

	/* text layer */
	segaic16_tilemap_draw(screen, bitmap, cliprect, 0, SEGAIC16_TILEMAP_TEXT, 0, 0x04);
	segaic16_tilemap_draw(screen, bitmap, cliprect, 0, SEGAIC16_TILEMAP_TEXT, 1, 0x08);

	/* draw the sprites */
	segaic16_sprites_draw(screen, bitmap, cliprect, 0);
	return 0;
}
