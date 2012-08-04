/***************************************************************************

    Sega Y-board hardware

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
#include "includes/segas16.h"


/*************************************
 *
 *  Video startup
 *
 *************************************/

VIDEO_START( yboard )
{
	segaybd_state *state = machine.driver_data<segaybd_state>();

	/* compute palette info */
	segaic16_palette_init(0x2000);

	/* allocate a bitmap for the yboard layer */
	state->m_tmp_bitmap = auto_bitmap_ind16_alloc(machine, 512, 512);

	/* initialize the rotation layer */
	segaic16_rotate_init(machine, 0, SEGAIC16_ROTATE_YBOARD, 0x000);

	state->save_item(NAME(*state->m_tmp_bitmap));
}



/*************************************
 *
 *  Video update
 *
 *************************************/

SCREEN_UPDATE_IND16( yboard )
{
	segaybd_state *state = screen.machine().driver_data<segaybd_state>();
	rectangle yboard_clip;

	/* if no drawing is happening, fill with black and get out */
	if (!segaic16_display_enable)
	{
		bitmap.fill(get_black_pen(screen.machine()), cliprect);
		return 0;
	}

	/* draw the yboard sprites */
	yboard_clip.set(0, 511, 0, 511);
	segaic16_sprites_draw(screen, *state->m_tmp_bitmap, yboard_clip, 1);

	/* apply rotation */
	segaic16_rotate_draw(screen.machine(), 0, bitmap, cliprect, state->m_tmp_bitmap);

	/* draw the 16B sprites */
	segaic16_sprites_draw(screen, bitmap, cliprect, 0);
	return 0;
}
