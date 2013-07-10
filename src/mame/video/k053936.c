/* */

#include "emu.h"
#include "k053936.h"

#define VERBOSE 0
#define LOG(x) do { if (VERBOSE) logerror x; } while (0)



/***************************************************************************/
/*                                                                         */
/*                                 053936                                  */
/*                                                                         */
/***************************************************************************/

#define K053936_MAX_CHIPS 2

static int K053936_offset[K053936_MAX_CHIPS][2];
static int K053936_wraparound[K053936_MAX_CHIPS];

// there is another implementation of this in  machine/konamigx.c (!)
//  why?

static void K053936_zoom_draw(int chip,UINT16 *ctrl,UINT16 *linectrl, bitmap_ind16 &bitmap,const rectangle &cliprect,tilemap_t *tmap,int flags,UINT32 priority, int glfgreat_hack)
{
	if (!tmap)
		return;

	if (ctrl[0x07] & 0x0040)
	{
		UINT32 startx,starty;
		int incxx,incxy;
		rectangle my_clip;
		int y,maxy;

		// Racin' Force will get to here if glfgreat_hack is enabled, and it ends
		// up setting a maximum y value of '13', thus causing nothing to be drawn.
		// It looks like the roz output should be flipped somehow as it seems to be
		// displaying the wrong areas of the tilemap and is rendered upside down,
		// although due to the additional post-processing the voxel renderer performs
		// it's difficult to know what the output SHOULD be.  (hold W in Racin' Force
		// to see the chip output)

		if (((ctrl[0x07] & 0x0002) && ctrl[0x09]) && (glfgreat_hack))   /* wrong, but fixes glfgreat */
		{
			my_clip.min_x = ctrl[0x08] + K053936_offset[chip][0]+2;
			my_clip.max_x = ctrl[0x09] + K053936_offset[chip][0]+2 - 1;
			if (my_clip.min_x < cliprect.min_x)
				my_clip.min_x = cliprect.min_x;
			if (my_clip.max_x > cliprect.max_x)
				my_clip.max_x = cliprect.max_x;

			y = ctrl[0x0a] + K053936_offset[chip][1]-2;
			if (y < cliprect.min_y)
				y = cliprect.min_y;
			maxy = ctrl[0x0b] + K053936_offset[chip][1]-2 - 1;
			if (maxy > cliprect.max_y)
				maxy = cliprect.max_y;
		}
		else
		{
			my_clip.min_x = cliprect.min_x;
			my_clip.max_x = cliprect.max_x;

			y = cliprect.min_y;
			maxy = cliprect.max_y;
		}

		while (y <= maxy)
		{
			UINT16 *lineaddr = linectrl + 4*((y - K053936_offset[chip][1]) & 0x1ff);
			my_clip.min_y = my_clip.max_y = y;



			startx = 256 * (INT16)(lineaddr[0] + ctrl[0x00]);
			starty = 256 * (INT16)(lineaddr[1] + ctrl[0x01]);
			incxx  =       (INT16)(lineaddr[2]);
			incxy  =       (INT16)(lineaddr[3]);

			if (ctrl[0x06] & 0x8000) incxx *= 256;
			if (ctrl[0x06] & 0x0080) incxy *= 256;

			startx -= K053936_offset[chip][0] * incxx;
			starty -= K053936_offset[chip][0] * incxy;

			tmap->draw_roz(bitmap, my_clip, startx << 5,starty << 5,
					incxx << 5,incxy << 5,0,0,
					K053936_wraparound[chip],
					flags,priority);

			y++;
		}
	}
	else    /* "simple" mode */
	{
		UINT32 startx,starty;
		int incxx,incxy,incyx,incyy;

		startx = 256 * (INT16)(ctrl[0x00]);
		starty = 256 * (INT16)(ctrl[0x01]);
		incyx  =       (INT16)(ctrl[0x02]);
		incyy  =       (INT16)(ctrl[0x03]);
		incxx  =       (INT16)(ctrl[0x04]);
		incxy  =       (INT16)(ctrl[0x05]);

		if (ctrl[0x06] & 0x4000) { incyx *= 256; incyy *= 256; }
		if (ctrl[0x06] & 0x0040) { incxx *= 256; incxy *= 256; }

		startx -= K053936_offset[chip][1] * incyx;
		starty -= K053936_offset[chip][1] * incyy;

		startx -= K053936_offset[chip][0] * incxx;
		starty -= K053936_offset[chip][0] * incxy;

		tmap->draw_roz(bitmap, cliprect, startx << 5,starty << 5,
				incxx << 5,incxy << 5,incyx << 5,incyy << 5,
				K053936_wraparound[chip],
				flags,priority);
	}

#if 0
if (machine.input().code_pressed(KEYCODE_D))
	popmessage("%04x %04x %04x %04x\n%04x %04x %04x %04x\n%04x %04x %04x %04x\n%04x %04x %04x %04x",
			ctrl[0x00],
			ctrl[0x01],
			ctrl[0x02],
			ctrl[0x03],
			ctrl[0x04],
			ctrl[0x05],
			ctrl[0x06],
			ctrl[0x07],
			ctrl[0x08],
			ctrl[0x09],
			ctrl[0x0a],
			ctrl[0x0b],
			ctrl[0x0c],
			ctrl[0x0d],
			ctrl[0x0e],
			ctrl[0x0f]);
#endif
}


void K053936_0_zoom_draw(bitmap_ind16 &bitmap,const rectangle &cliprect,tilemap_t *tmap,int flags,UINT32 priority, int glfgreat_hack)
{
	UINT16 *ctrl = reinterpret_cast<UINT16 *>(tmap->machine().root_device().memshare("k053936_0_ctrl")->ptr());
	UINT16 *linectrl = reinterpret_cast<UINT16 *>(tmap->machine().root_device().memshare("k053936_0_line")->ptr());
	K053936_zoom_draw(0,ctrl,linectrl,bitmap,cliprect,tmap,flags,priority, glfgreat_hack);
}

void K053936_wraparound_enable(int chip, int status)
{
	K053936_wraparound[chip] = status;
}


void K053936_set_offset(int chip, int xoffs, int yoffs)
{
	K053936_offset[chip][0] = xoffs;
	K053936_offset[chip][1] = yoffs;
}



