/***************************************************************************

    Sega Hang On hardware

****************************************************************************

    Copyright Aaron Giles
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are
    met:

        * Redistributions of source code must retain the above copyright
          notice, this list of conditions and the following disclaimer.
        * Redistributions in binary form must reproduce the above copyright
          notice, this list of conditions and the following disclaimer in
          the documentation and/or other materials provided with the
          distribution.
        * Neither the name 'MAME' nor the names of its contributors may be
          used to endorse or promote products derived from this software
          without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY AARON GILES ''AS IS'' AND ANY EXPRESS OR
    IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL AARON GILES BE LIABLE FOR ANY DIRECT,
    INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
    SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
    HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
    STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
    IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
    POSSIBILITY OF SUCH DAMAGE.

***************************************************************************/

#include "emu.h"
#include "includes/segahang.h"



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
	segaic16_tilemap_init(machine, 0, SEGAIC16_TILEMAP_HANGON, 0x000, 0, 2);

	/* initialize the road */
	segaic16_road_init(machine, 0, SEGAIC16_ROAD_HANGON, 0x038, 0x7c0, 0x7c0, 0);
}


VIDEO_START( sharrier )
{
	/* compute palette info */
	segaic16_palette_init(0x800);

	/* initialize the tile/text layers */
	segaic16_tilemap_init(machine, 0, SEGAIC16_TILEMAP_HANGON, 0x000, 0, 2);

	/* initialize the road */
	segaic16_road_init(machine, 0, SEGAIC16_ROAD_SHARRIER, 0x038, 0x7c0, 0x7c0, 0);
}



/*************************************
 *
 *  Video update
 *
 *************************************/

SCREEN_UPDATE_IND16( hangon )
{
	/* if no drawing is happening, fill with black and get out */
	if (!segaic16_display_enable)
	{
		bitmap.fill(get_black_pen(screen.machine()), cliprect);
		return 0;
	}

	/* reset priorities */
	screen.machine().priority_bitmap.fill(0, cliprect);

	/* draw the low priority road layer */
	segaic16_road_draw(0, bitmap, cliprect, SEGAIC16_ROAD_BACKGROUND);

	/* draw background */
	segaic16_tilemap_draw(screen, bitmap, cliprect, 0, SEGAIC16_TILEMAP_BACKGROUND, 0, 0x01);
	segaic16_tilemap_draw(screen, bitmap, cliprect, 0, SEGAIC16_TILEMAP_BACKGROUND, 1, 0x02);

	/* draw foreground */
	segaic16_tilemap_draw(screen, bitmap, cliprect, 0, SEGAIC16_TILEMAP_FOREGROUND, 0, 0x02);
	segaic16_tilemap_draw(screen, bitmap, cliprect, 0, SEGAIC16_TILEMAP_FOREGROUND, 1, 0x04);

	/* draw the high priority road */
	segaic16_road_draw(0, bitmap, cliprect, SEGAIC16_ROAD_FOREGROUND);

	/* text layer */
	/* note that we inflate the priority of the text layer to prevent sprites */
	/* from drawing over the high scores */
	segaic16_tilemap_draw(screen, bitmap, cliprect, 0, SEGAIC16_TILEMAP_TEXT, 0, 0x08);
	segaic16_tilemap_draw(screen, bitmap, cliprect, 0, SEGAIC16_TILEMAP_TEXT, 1, 0x08);

	/* draw the sprites */
	segaic16_sprites_draw(screen, bitmap, cliprect, 0);
	return 0;
}

