/***************************************************************************

  nc.c

  Functions to emulate the video hardware of the Amstrad PCW.

***************************************************************************/

#include "emu.h"
#include "includes/nc.h"
#include "machine/ram.h"

/***************************************************************************
  Start the video hardware emulation.
***************************************************************************/

VIDEO_START( nc )
{
}

/* two colours */
static const unsigned short nc_colour_table[NC_NUM_COLOURS] =
{
	0, 1,2,3
};

/* black/white */
static const rgb_t nc_palette[NC_NUM_COLOURS] =
{
	MAKE_RGB(0x060, 0x060, 0x060),
	MAKE_RGB(0x000, 0x000, 0x000),
	MAKE_RGB(0x080, 0x0a0, 0x060),
    MAKE_RGB(0x000, 0x000, 0x000)
};


/* Initialise the palette */
PALETTE_INIT( nc )
{
	palette_set_colors(machine, 0, nc_palette, ARRAY_LENGTH(nc_palette));
}


void nc200_video_set_backlight(running_machine &machine, int state)
{
	nc_state *drvstate = machine.driver_data<nc_state>();
	drvstate->m_nc200_backlight = state;
}


/***************************************************************************
  Draw the game screen in the given bitmap_ind16.
  Do NOT call osd_update_display() from this function,
  it will be called by the main emulation engine.
***************************************************************************/
SCREEN_UPDATE_IND16( nc )
{
	nc_state *state = screen.machine().driver_data<nc_state>();
	int y;
	int b;
	int x;
	int height, width;
	int pens[2];

    if (state->m_type==NC_TYPE_200)
    {
        height = NC200_SCREEN_HEIGHT;
        width = NC200_SCREEN_WIDTH;

		if (state->m_nc200_backlight)
		{
			pens[0] = 2;
			pens[1] = 3;
		}
		else
		{
			pens[0] = 0;
			pens[1] = 1;
		}
    }
    else
    {
		height = NC_SCREEN_HEIGHT;
		width = NC_SCREEN_WIDTH;
		pens[0] = 2;
		pens[1] = 3;
	}


    for (y=0; y<height; y++)
    {
		int by;
		/* 64 bytes per line */
		char *line_ptr = ((char*)screen.machine().device<ram_device>(RAM_TAG)->pointer()) + state->m_display_memory_start + (y<<6);

		x = 0;
		for (by=0; by<width>>3; by++)
		{
			int px;
			unsigned char byte;

			byte = line_ptr[0];

			px = x;
			for (b=0; b<8; b++)
			{
				bitmap.pix16(y, px) = pens[(byte>>7) & 0x01];
				byte = byte<<1;
				px++;
			}

			x = px;

			line_ptr = line_ptr+1;
		}
	}
	return 0;
}

