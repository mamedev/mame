/*********************************************************************

    pc_video.c

    PC Video code

*********************************************************************/

#include "emu.h"
#include "memconv.h"
//#include "includes/crtc6845.h"
#include "video/pc_video.h"



/***************************************************************************

    Local variables

***************************************************************************/

static pc_video_update_proc (*pc_choosevideomode)(running_machine *machine, int *width, int *height);
static int pc_anythingdirty;
static int pc_current_height;
static int pc_current_width;
static const UINT16 dummy_palette[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 };



/**************************************************************************/

static STATE_POSTLOAD( pc_video_postload )
{
	pc_anythingdirty = 1;
	pc_current_height = -1;
	pc_current_width = -1;
}



void pc_video_start(running_machine *machine,
	pc_video_update_proc (*choosevideomode)(running_machine *machine, int *width, int *height),
	size_t vramsize)
{
	pc_choosevideomode = choosevideomode;
	pc_anythingdirty = 1;
	pc_current_height = -1;
	pc_current_width = -1;
	machine->generic.tmpbitmap = NULL;

	if (vramsize)
	{
		video_start_generic_bitmapped(machine);
	}

	state_save_register_postload(machine, pc_video_postload, NULL);
}



VIDEO_UPDATE( pc_video )
{
	UINT32 rc = 0;
	int w = 0, h = 0;
	pc_video_update_proc video_update;

	video_update = pc_choosevideomode(screen->machine, &w, &h);

	if (video_update)
	{
		if ((pc_current_width != w) || (pc_current_height != h))
		{
			int width = screen->width();
			int height = screen->height();

			pc_current_width = w;
			pc_current_height = h;
			pc_anythingdirty = 1;

			if (pc_current_width > width)
				pc_current_width = width;
			if (pc_current_height > height)
				pc_current_height = height;

			if ((pc_current_width > 100) && (pc_current_height > 100))
				screen->set_visible_area(0, pc_current_width-1, 0, pc_current_height-1);

			bitmap_fill(bitmap, cliprect, 0);
		}

		video_update(screen->machine->generic.tmpbitmap ? screen->machine->generic.tmpbitmap : bitmap);

		if (screen->machine->generic.tmpbitmap)
		{
			copybitmap(bitmap, screen->machine->generic.tmpbitmap, 0, 0, 0, 0, cliprect);
			if (!pc_anythingdirty)
				rc = UPDATE_HAS_NOT_CHANGED;
			pc_anythingdirty = 0;
		}
	}
	return rc;
}
