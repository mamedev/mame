/***************************************************************************

    Sega Y-board hardware

***************************************************************************/

#include "driver.h"
#include "segaic16.h"
#include "includes/system16.h"



/*************************************
 *
 *  Statics
 *
 *************************************/

static bitmap_t *yboard_bitmap;



/*************************************
 *
 *  Video startup
 *
 *************************************/

VIDEO_START( yboard )
{
	/* compute palette info */
	segaic16_palette_init(0x2000);

	/* allocate a bitmap for the yboard layer */
	yboard_bitmap = auto_bitmap_alloc(machine, 512, 512, BITMAP_FORMAT_INDEXED16);

	/* initialize the sprites */
	segaic16_sprites_init(machine, 0, SEGAIC16_SPRITES_YBOARD_16B, 0x800, 0);
	segaic16_sprites_init(machine, 1, SEGAIC16_SPRITES_YBOARD, 0x1000, 0);

	/* initialize the rotation layer */
	segaic16_rotate_init(machine, 0, SEGAIC16_ROTATE_YBOARD, 0x000);
}



/*************************************
 *
 *  Video update
 *
 *************************************/

VIDEO_UPDATE( yboard )
{
	rectangle yboard_clip;

	/* if no drawing is happening, fill with black and get out */
	if (!segaic16_display_enable)
	{
		bitmap_fill(bitmap, cliprect, get_black_pen(screen->machine));
		return 0;
	}

	/* draw the yboard sprites */
	yboard_clip.min_x = yboard_clip.min_y = 0;
	yboard_clip.max_x = yboard_clip.max_y = 511;
	segaic16_sprites_draw(screen, yboard_bitmap, &yboard_clip, 1);

	/* apply rotation */
	segaic16_rotate_draw(screen->machine, 0, bitmap, cliprect, yboard_bitmap);

	/* draw the 16B sprites */
	segaic16_sprites_draw(screen, bitmap, cliprect, 0);
	return 0;
}
